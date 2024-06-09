#include "taskdep.h"
#include "proto/taskdep/message.h"
#include <workflow/WFServer.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/Workflow.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
#include <functional>
#include <random>
#include <unordered_map>
#include <vector>

namespace jxtd
{
    namespace storage
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

            static std::string anonymous_mid = "0000aaaa";

            static uint64_t random_uint64_t()
            {
                std::random_device rd;
                std::mt19937_64 gen(rd());

                uint64_t min = 1;
                uint64_t max = std::numeric_limits<uint64_t>::max();

                std::uniform_int_distribution<uint64_t> dis(min, max);

                return dis(gen);
            }

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

            static void process(TaskdepTask *task, TaskdepServer *server_ctx, Datamanager &d_manager, std::mutex &task_mutex);

            StorageServer::StorageServer()
            {
                this->server_ctx = new TaskdepServer;
            }

            StorageServer::~StorageServer()
            {
                this->stop();
                delete this->server_ctx;
            }

            // server建立功能
            int StorageServer::setup(const TaskdepSetupContext &ctx)
            {
                if (!ctx.endpoint.check())
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
            int StorageServer::start(Datamanager &d_manager, std::mutex &task_mutex)
            {
                auto configs = HTTP_SERVER_PARAMS_DEFAULT;
                configs.peer_response_timeout = this->server_ctx->response_timeout;
                configs.request_size_limit = this->server_ctx->request_size_limit;
                this->server_ctx->server = new TaskdepWFServer(
                    &configs,
                    std::bind(process, std::placeholders::_1, this->server_ctx, std::ref(d_manager), std::ref(task_mutex)));
                if (this->server_ctx->ssl)
                    return this->server_ctx->server->start(
                        this->server_ctx->family,
                        this->server_ctx->endpoint.get_url().c_str(),
                        this->server_ctx->endpoint.get_port(),
                        this->server_ctx->cert.c_str(),
                        this->server_ctx->key.c_str());
                else
                    return this->server_ctx->server->start(
                        this->server_ctx->family,
                        this->server_ctx->endpoint.get_url().c_str(),
                        this->server_ctx->endpoint.get_port());
            }

            // server停止功能
            void StorageServer::stop()
            {
                if (this->server_ctx->server != nullptr)
                {
                    this->server_ctx->server->stop();
                    delete this->server_ctx->server;
                    this->server_ctx->server = nullptr;
                }
            }

            static void process(TaskdepTask *task, TaskdepServer *server_ctx, Datamanager &d_manager, std::mutex &task_mutex)
            {
                auto req = task->get_req();
                auto resp = task->get_resp();
                if(!d_manager.get_is_init())
                {
                    task->noreply();
                    return;
                }
                switch (req->get_type_src())
                {
                    case static_cast<uint32_t>(ADDR_TYPE::management):
                    {
                        if (req->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::storage) || req->get_object_management() == nullptr)
                        {
                            task->noreply();
                            return;
                        }
                        switch (req->get_type_msg())
                        {
                            case static_cast<uint32_t>(MSG_TYPE::neogiate):
                            {
                                if (req->get_engine() == nullptr)
                                {
                                    task->noreply();
                                    return;
                                }
                                resp->set_type(static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(MSG_TYPE::neogiate));
                                {
                                    std::unique_lock<std::mutex> lock(task_mutex);
                                    auto taskid = req->get_taskid();
                                    if(d_manager.task_exist(taskid))
                                    {
                                        resp->set_neogiate_status(static_cast<uint32_t>(Response::Neogiate_status::conflict));
                                        // errstr
                                        resp->add_neogiate_alternative(random_uint64_t());
                                    }   
                                    else
                                    {
                                        auto engine = req->get_engine();
                                        if(engine == nullptr)
                                        {
                                            // 或者设置什么错误
                                            task->noreply();
                                            return;
                                        }
                                        std::string mid;
                                        std::string name;
                                        if(!engine->binary.empty())
                                        {
                                            mid = anonymous_mid;
                                            name = std::to_string(taskid);
                                            d_manager.engine_push(mid, name, engine->binary);
                                        }
                                        else
                                        {
                                            mid = engine->id;
                                            name = engine->name;
                                        }
                                        d_manager.task_bind(taskid, std::make_pair(mid, name), engine->deps);
                                        if(!engine->extfiles.empty())
                                            d_manager.add_extfiles(taskid, engine->extfiles);
                                        resp->set_neogiate_status(static_cast<uint32_t>(Response::Neogiate_status::succeed));
                                    }
                                }
                                break;
                            }

                            case static_cast<uint32_t>(MSG_TYPE::terminate):
                            {
                                resp->set_type(static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(MSG_TYPE::terminate));
                                {
                                    std::unique_lock<std::mutex> lock(task_mutex);
                                    auto taskid = req->get_taskid();
                                    if(d_manager.task_exist(taskid))
                                    {
                                        auto engine = d_manager.get_task_engine(taskid);
                                        if(engine.first.first == anonymous_mid)
                                            d_manager.engine_remove(anonymous_mid, std::to_string(taskid));
                                        d_manager.task_unbind(taskid);
                                    }
                                    if(d_manager.extfiles_exist(taskid))
                                        d_manager.remove_extfiles(taskid);
                                }
                                break;
                            }

                            default:
                                task->noreply();
                                return;
                        };
                        // set_token
                        resp->set_taskid(req->get_taskid());
                        resp->set_object_management(*(req->get_object_management()));
                        break;
                    }
                    case static_cast<uint32_t>(ADDR_TYPE::cluster):
                    {
                        if(req->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::storage) || req->get_type_msg() != static_cast<uint32_t>(MSG_TYPE::done))
                        {
                            task->noreply();
                            return;
                        }
                        if(req->get_object_cluster() == nullptr)
                        {
                            task->noreply();
                            return;
                        }
                        // set_token
                        resp->set_taskid(req->get_taskid());
                        resp->set_type(static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(MSG_TYPE::done));
                        {
                            std::unique_lock<std::mutex> lock(task_mutex);
                            auto taskid = req->get_taskid();
                            if(d_manager.task_exist(taskid))
                            {
                                auto engine = d_manager.get_task_engine(taskid);
                                if(engine.first.first == anonymous_mid)
                                    d_manager.engine_remove(anonymous_mid, std::to_string(taskid));
                                d_manager.task_unbind(taskid);
                            }
                            if(d_manager.extfiles_exist(taskid))
                                d_manager.remove_extfiles(taskid);
                        }
                        resp->set_object_cluster(*(req->get_object_cluster()));
                        break;
                    }
                    default:
                        task->noreply();
                        return;
                }
                resp->set_object_storage(server_ctx->endpoint);
            }

        }
    }
}