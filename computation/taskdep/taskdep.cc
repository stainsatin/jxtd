#include "taskdep.h"
#include "proto/taskdep/message.h"
#include <workflow/WFServer.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/Workflow.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
#include <functional>
#include <unordered_map>
#include <vector>

#include "computation/infer/infer.h"

namespace jxtd
{
    namespace computation
    {
        namespace taskdep
        {
            using Request = ::jxtd::proto::taskdep::Request;
            using Response = ::jxtd::proto::taskdep::Response;
            using TaskdepTask = WFNetworkTask<Request, Response>;
            using TaskdepWFServer = WFServer<Request, Response>;
            using TaskdepTaskFactory = WFNetworkTaskFactory<Request, Response>;
            using MSG_TYPE = ::jxtd::proto::taskdep::MSG_TYPE;
            using ADDR_TYPE = ::jxtd::proto::taskdep::ADDR_TYPE;
            using Trigger = ::jxtd::proto::taskdep::Request::Attr_tigger;
            using Deploy_status = ::jxtd::proto::taskdep::Response::Deploy_status;
            using Deploy_taskstatus = ::jxtd::proto::taskdep::Response::Deploy_taskstatus;

            static void process(TaskdepTask *task, TaskdepServer *server, std::string name, Taskmanager &t_manager, Servermanager &s_manager);

            struct TaskdepServer
            {
                Endpoint endpoint;
                bool ssl;
                TaskdepWFServer *server = nullptr;
                int family;
                std::string ca;
                std::string cert;
                std::string key;
                int response_timeout;
                int request_size_limit;
            };

            ClusterServer::ClusterServer()
            {
                this->server_ctx = new TaskdepServer;
            }

            ClusterServer::~ClusterServer()
            {
                this->stop();
                delete this->server_ctx;
            }

            // server建立功能
            int ClusterServer::setup(const TaskdepSetupContext& ctx)
            {
                if(!ctx.endpoint.check())
                    return -1;
                this->server_ctx->endpoint = ctx.endpoint;
                this->server_ctx->family = ctx.family;
                this->server_ctx->ssl = ctx.ssl;
                this->server_ctx->ca = ctx.ca;
                this->server_ctx->cert = ctx.cert;
                this->server_ctx->key = ctx.key;
                this->server_ctx->response_timeout = ctx.response_timeout;
                this->server_ctx->request_size_limit = ctx.request_size_limit;
                return 0;
            }

            // server启动功能
            int ClusterServer::start(std::string name, Taskmanager &t_manager, Servermanager &s_manager)
            {
                auto configs = HTTP_SERVER_PARAMS_DEFAULT;
                configs.peer_response_timeout = this->server_ctx->response_timeout;
                configs.request_size_limit = this->server_ctx->request_size_limit;
                this->server_ctx->server = new TaskdepWFServer(
                    &configs,
                    std::bind(process, std::placeholders::_1, this->server_ctx, name, std::ref(t_manager), std::ref(s_manager))
                );
                if(this->server_ctx->ssl)
                    return this->server_ctx->server->start(
                        this->server_ctx->family,
                        this->server_ctx->endpoint.get_url().c_str(),
                        this->server_ctx->endpoint.get_port(),
                        this->server_ctx->cert.c_str(),
                        this->server_ctx->key.c_str()
                    );
                else
                    return this->server_ctx->server->start(
                        this->server_ctx->family,
                        this->server_ctx->endpoint.get_url().c_str(),
                        this->server_ctx->endpoint.get_port()
                    );
            }

            // server停止功能
            void ClusterServer::stop()
            {
                if(this->server_ctx->server != nullptr)
                {
                    this->server_ctx->server->stop();
                    delete this->server_ctx->server;
                    this->server_ctx->server = nullptr;
                }
            }

            void process(TaskdepTask *task, TaskdepServer *server, std::string name, Taskmanager &t_manager, Servermanager &s_manager)
            {
                auto req = task->get_req();
                auto resp = task->get_resp();
                if(req->get_type_src() != static_cast<uint32_t>(ADDR_TYPE::management) || req->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::cluster) || req->get_object_management() == nullptr)
                {
                    task->noreply();
                    return;
                }

                uint64_t taskid = req->get_taskid();
                switch(req->get_type_msg())
                {
                    case static_cast<uint32_t>(MSG_TYPE::deploy):
                    {
                        resp->set_type(static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(MSG_TYPE::deploy));
                        if(req->get_attr() == nullptr || req->get_datasource_src() == nullptr || req->get_datasource_dest() == nullptr)
                        {
                            task->noreply();
                            return;
                        }
                        auto src = req->get_datasource_src();
                        std::string type = src->type;
                        std::string input_path;
                        std::string input_data;
                        if(type == "data")
                            input_data = src->input;
                        else if(type == "endpoint")
                            input_path = std::move(src->input);
                        else
                        {
                            resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::illegal));
                            break;
                        }
                        std::string output_path;
                        auto dest = req->get_datasource_dest();
                        auto attr = req->get_attr();
                        if(!attr->virtstorage)
                            output_path = dest->output;
                        auto period = attr->period;
                        int judge;
                        if(period == nullptr)
                            judge = t_manager.deploy_task(taskid, input_path, input_data, output_path, dest->directorypolicy, dest->emptypolicy, 0, 0, 0, {});
                        else
                            judge = t_manager.deploy_task(taskid, input_path, input_data, output_path, dest->directorypolicy, dest->emptypolicy, period->interval, period->resultpolicy, period->triggerpolicy, period->backupid);
                        if(judge == -1)
                        {
                            resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::illegal));
                            break;
                        }
                        switch(attr->trigger)
                        {
                            case static_cast<uint32_t>(Trigger::create):
                            {
                                auto sub_task = t_manager.submit(s_manager, name, taskid);
                                if(sub_task != nullptr)
                                {
                                    resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::succeed));
                                    resp->set_deploy_taskstatus(static_cast<uint32_t>(Deploy_taskstatus::triggered));
                                    Workflow::start_series_work(sub_task, nullptr);
                                }
                                else
                                {
                                    t_manager.remove_task(taskid, false);
                                    resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::err));
                                }
                                break;
                            }
                            case static_cast<uint32_t>(Trigger::deps):
                                if(t_manager.add_deps(taskid, attr->deps) == -1)
                                {
                                    resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::depsbroken));
                                    t_manager.remove_task(taskid, false);
                                }
                                else
                                {
                                    resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::succeed));
                                    resp->set_deploy_taskstatus(static_cast<uint32_t>(Deploy_taskstatus::deployed));
                                }
                                break;
                            default:
                                resp->set_deploy_status(static_cast<uint32_t>(Deploy_status::succeed));
                                resp->set_deploy_taskstatus(static_cast<uint32_t>(Deploy_taskstatus::deployed));
                                break;
                        }
                        break;
                    }
                    case static_cast<uint32_t>(MSG_TYPE::terminate):
                        resp->set_type(static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(MSG_TYPE::terminate));
                        t_manager.remove_task(taskid, false); // 管理单元负责处理依赖的任务
                        break; // terminate没有设置resp
                    case static_cast<uint32_t>(MSG_TYPE::trigger):
                    {
                        resp->set_type(static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(MSG_TYPE::trigger));
                        auto sub_task = t_manager.submit(s_manager, name, taskid);
                        if(sub_task != nullptr)
                            Workflow::start_series_work(sub_task, nullptr); // trigger也没有设置resp
                        break;
                    }
                    default:
                        task->noreply();
                        return;
                }
                // token
                resp->set_taskid(taskid);
                resp->set_object_management(*(req->get_object_management()));
                resp->set_object_cluster(server->endpoint);
            }
        }
    }
}