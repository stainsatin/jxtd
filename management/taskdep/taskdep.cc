#include "taskdep.h"
#include <string>
#include <functional>
#include <iostream>
#include <workflow/WFTaskFactory.h>
#include <workflow/Workflow.h>
#include <workflow/WFFacilities.h>

namespace jxtd
{
    namespace management
    {
        namespace taskdep
        {
            using TaskdepTask = WFNetworkTask<Request, Response>;
            using Endpoint = ::jxtd::proto::taskdep::Endpoint;
            using TaskdepTaskFactory = WFNetworkTaskFactory<Request, Response>;
            using MSG_TYPE = ::jxtd::proto::taskdep::MSG_TYPE;
            using ADDR_TYPE = ::jxtd::proto::taskdep::ADDR_TYPE;

            struct TaskdepTaskContext
            {
                TaskdepTask *task;
                bool success;
            };
            using TaskdepTaskContext = struct TaskdepTaskContext;

            static TaskdepTask *create_neogiate_task_repeater(WFRepeaterTask *entrypoint, TaskdepClientContext *client_ctx, uint64_t &taskid, const Engine &engine, uint64_t &result)
            {
                auto task_ctx = (TaskdepTaskContext*)series_of(entrypoint)->get_context();
                if(task_ctx->success)
                    return nullptr;
                auto forward_task = TaskdepTaskFactory::create_client_task(
                    client_ctx->ssl ? TT_TCP_SSL:TT_TCP,
                    client_ctx->storage.get_url(), 
                    client_ctx->storage.get_port(),
                    1,
                    [&result, task_ctx, &taskid](TaskdepTask* task){
                        if(task->get_state() != WFT_STATE_SUCCESS)
                        {
                            task_ctx->success = false;
                            return;
                        }
                        auto resp = task->get_resp();
                        if(resp->get_type_src() != static_cast<uint32_t>(ADDR_TYPE::storage) || resp->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::management) || resp->get_type_msg() != static_cast<uint32_t>(MSG_TYPE::neogiate) || resp->get_taskid() != taskid)
                        {
                            task_ctx->success = true;
                            return;
                        }
                        switch(resp->get_neogiate()->status)
                        {
                            case static_cast<uint32_t>(Response::Neogiate_status::succeed):
                                result = taskid;
                                task_ctx->success = true;
                                break;
                            case static_cast<uint32_t>(Response::Neogiate_status::conflict):
                                if (resp->get_neogiate()->alternative.empty())
                                {
                                    task_ctx->success = true;
                                    break;
                                }
                                else
                                {
                                    taskid = resp->get_neogiate()->alternative.front();
                                    task_ctx->success = false;
                                    break;
                                }
                            default:
                                task_ctx->success = true;
                                break;
                        }
                    }
                );
                auto req = forward_task -> get_req();
                req->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(MSG_TYPE::neogiate));
                req->set_object_management(client_ctx->management);
                req->set_object_storage(client_ctx->storage);
                req->set_token(client_ctx->token);
                req->set_taskid(taskid);
                req->set_engine_binary(engine.binary);
                req->set_engine_id_name(engine.id, engine.name);
                for(const auto &dep : engine.deps)
                    req->add_engine_dep(dep);
                for(const auto &extfile : engine.extfiles)
                    req->add_engine_extfile(extfile);
                return forward_task;
            }

            static TaskdepTask *create_deploy_task_repeater(WFRepeaterTask *entrypoint, TaskdepClientContext *client_ctx, uint64_t taskid, const Datasource &datasource, const Attr &attr, int &busy_retry_max, int &result)
            {
                auto task_ctx = (TaskdepTaskContext*)series_of(entrypoint)->get_context();
                if(task_ctx->success)
                    return nullptr;
                auto forward_task = TaskdepTaskFactory::create_client_task(
                    client_ctx->ssl ? TT_TCP_SSL:TT_TCP,
                    client_ctx->computation.get_url(), 
                    client_ctx->computation.get_port(),
                    1,
                    [&busy_retry_max, &result, task_ctx, client_ctx, &taskid](TaskdepTask* task){
                        if(task->get_state() != WFT_STATE_SUCCESS)
                        {
                            task_ctx->success = false;
                            return;
                        }
                        auto resp = task->get_resp();
                        if(resp->get_type_src() != static_cast<uint32_t>(ADDR_TYPE::cluster) || resp->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::management) || resp->get_type_msg() != static_cast<uint32_t>(MSG_TYPE::deploy) || resp->get_taskid() != taskid)
                        {
                            task_ctx->success = true;
                            return;
                        }
                        switch(resp->get_deploy()->status)
                        {
                            case static_cast<uint32_t>(Response::Deploy_status::succeed):
                                result = resp->get_deploy()->taskstatus;
                                task_ctx->success = true;
                                break;
                            case static_cast<uint32_t>(Response::Deploy_status::busy):
                                if(busy_retry_max == 0)
                                    task_ctx->success = true;
                                task_ctx->success = false;
                                busy_retry_max--;
                                break;
                            default:
                                task_ctx->success = true;
                                break;
                        }
                    }
                );
                auto req = forward_task -> get_req();
                req->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(MSG_TYPE::deploy));
                req->set_object_management(client_ctx->management);
                req->set_object_cluster(client_ctx->computation);
                req->set_token(client_ctx->token);
                req->set_taskid(taskid);
                req->set_datasource(datasource.src, datasource.dest);
                req->set_attr_master(attr.trigger, attr.virtstorage);
                if(attr.period != nullptr)
                    req->set_attr_period(attr.period->interval, attr.period->resultpolicy, attr.period->triggerpolicy, attr.period->backupid);
                for(const auto &dep : attr.deps)
                    req->add_attr_dep(dep);
                return forward_task;
            }

            ManagementClient::ManagementClient()
            {
                this->client_ctx = new TaskdepClientContext;
            }

            ManagementClient::~ManagementClient()
            {
                delete this->client_ctx;
                this->client_ctx = nullptr;
            }

            int ManagementClient::setup(const TaskdepClientContext &ctx)
            {
                this->client_ctx->storage = ctx.storage;
                this->client_ctx->computation = ctx.computation;
                this->client_ctx->management = ctx.management;
                this->client_ctx->token = ctx.token;
                this->client_ctx->ssl = ctx.ssl;
                return 0;
            }

            uint64_t ManagementClient::create_neogiate_task(uint64_t taskid, const Engine &engine)
            {
                uint64_t result = 0;
                auto task = TaskdepTaskFactory::create_client_task (
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    this->client_ctx->storage.get_url(),
                    this->client_ctx->storage.get_port(),
                    1,
                    [&taskid, &engine, &result, this](TaskdepTask *task)
                    {
                        if(task->get_state() != WFT_STATE_SUCCESS)
                            return;
                        auto resp = task->get_resp();
                        if(resp->get_type_src() != static_cast<uint32_t>(ADDR_TYPE::storage) || resp->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::management) || resp->get_type_msg() != static_cast<uint32_t>(MSG_TYPE::neogiate) || resp->get_taskid() != taskid)
                            return;
                        switch (resp->get_neogiate()->status)
                        {
                            case static_cast<uint32_t>(Response::Neogiate_status::succeed):
                            {
                                result = taskid;
                                break;
                                // 先不处理errstr
                            }
                            case static_cast<uint32_t>(Response::Neogiate_status::conflict):
                            {
                                if (resp->get_neogiate()->alternative.empty())
                                    break;
                                else
                                {
                                    taskid = resp->get_neogiate()->alternative.front();
                                    series_of(task)->push_back(WFTaskFactory::create_repeater_task(
                                        std::bind(create_neogiate_task_repeater, std::placeholders::_1, this->client_ctx, std::ref(taskid), std::cref(engine), std::ref(result)), nullptr)
                                    );
                                    break;
                                }
                            }
                            default:
                                break;
                        }
                    }
                );
                auto req = task->get_req();
                req->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(MSG_TYPE::neogiate));
                req->set_object_management(this->client_ctx->management);
                req->set_object_storage(this->client_ctx->storage);
                req->set_token(this->client_ctx->token);
                req->set_taskid(taskid);
                req->set_engine_binary(engine.binary);
                req->set_engine_id_name(engine.id, engine.name);
                for(const auto &dep : engine.deps)
                    req->add_engine_dep(dep);
                for(const auto &extfile : engine.extfiles)
                    req->add_engine_extfile(extfile);
                auto ctx = new TaskdepTaskContext;
                ctx->task = task;
                ctx->success = false;
                WFFacilities::WaitGroup group(1);
                auto series = Workflow::create_series_work(task, [&group](const SeriesWork* series) {
                    auto ctx = (TaskdepTaskContext*)(series -> get_context());
                    delete ctx;
                    group.done();
                });
                series -> set_context(ctx);
                series -> start();
                group.wait();
                return result;
            }

            int ManagementClient::create_deploy_task(uint64_t taskid, const Datasource &datasource, const Attr &attr, int busy_retry_max)
            {
                int result = -1;
                auto task = TaskdepTaskFactory::create_client_task (
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    this->client_ctx->computation.get_url(),
                    this->client_ctx->computation.get_port(),
                    1,
                    [&taskid, &datasource, &attr, &busy_retry_max, &result, this](TaskdepTask *task)
                    {
                        if(task->get_state() != WFT_STATE_SUCCESS)
                            return;
                        auto resp = task->get_resp();
                        if(resp->get_type_src() != static_cast<uint32_t>(ADDR_TYPE::cluster) || resp->get_type_dest() != static_cast<uint32_t>(ADDR_TYPE::management) || resp->get_type_msg() != static_cast<uint32_t>(MSG_TYPE::deploy) || resp->get_taskid() != taskid)
                            return;
                        switch (resp->get_deploy()->status)
                        {
                            case static_cast<uint32_t>(Response::Deploy_status::succeed):
                                result = resp->get_deploy()->taskstatus;
                                break;
                            case static_cast<uint32_t>(Response::Deploy_status::busy):
                                {
                                    series_of(task) -> push_back(WFTaskFactory::create_repeater_task(
                                        std::bind(create_deploy_task_repeater, std::placeholders::_1, this->client_ctx, std::ref(taskid), std::cref(datasource), std::cref(attr), std::ref(busy_retry_max), std::ref(result)), nullptr)
                                    );
                                }
                                break;
                            default:
                                break;
                        }
                    } );
                auto req = task->get_req();
                req->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(MSG_TYPE::deploy));
                req->set_object_management(this->client_ctx->management);
                req->set_object_cluster(this->client_ctx->computation);
                req->set_token(this->client_ctx->token);
                req->set_taskid(taskid);
                req->set_datasource(datasource.src, datasource.dest);
                req->set_attr_master(attr.trigger, attr.virtstorage);
                if(attr.period != nullptr)
                    req->set_attr_period(attr.period->interval, attr.period->resultpolicy, attr.period->triggerpolicy, attr.period->backupid);
                for(const auto &dep : attr.deps)
                    req->add_attr_dep(dep);
                auto ctx = new TaskdepTaskContext;
                ctx->task = task;
                ctx->success = false;
                WFFacilities::WaitGroup group(1);
                auto series = Workflow::create_series_work(task, [&group](const SeriesWork* series){
                    auto ctx = (TaskdepTaskContext*)(series -> get_context());
                    delete ctx;
                    group.done();
                });
                series -> set_context(ctx);
                series -> start();
                group.wait();
                return result;
            }

            int ManagementClient::create_terminate_task(uint64_t taskid)
            {
                WFFacilities::WaitGroup group(1);
                auto task1 = TaskdepTaskFactory::create_client_task (
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    this->client_ctx->computation.get_url(),
                    this->client_ctx->computation.get_port(),
                    1,
                    nullptr
                );

                auto task2 = TaskdepTaskFactory::create_client_task (
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    this->client_ctx->storage.get_url(),
                    this->client_ctx->storage.get_port(),
                    1,
                    nullptr
                );
                auto req1 = task1->get_req();
                req1->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(MSG_TYPE::terminate));
                req1->set_object_management(this->client_ctx->management);
                req1->set_object_cluster(this->client_ctx->storage);
                req1->set_token(this->client_ctx->token);
                req1->set_taskid(taskid);

                auto req2 = task2->get_req();
                req2->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(MSG_TYPE::terminate));
                req2->set_object_management(this->client_ctx->management);
                req2->set_object_storage(this->client_ctx->storage);
                req2->set_token(this->client_ctx->token);
                req2->set_taskid(taskid);

                SeriesWork *series = Workflow::create_series_work(task2, [&group](const SeriesWork *task) {group.done();});
                series->push_back(task1);
	            series->start();
                group.wait();
                return 0;
            }

            int ManagementClient::create_trigger_task(uint64_t taskid)
            {
                WFFacilities::WaitGroup group(1);
                auto task = TaskdepTaskFactory::create_client_task (
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    this->client_ctx->computation.get_url(),
                    this->client_ctx->computation.get_port(),
                    1,
                    nullptr
                );
                auto req = task->get_req();
                req->set_type(static_cast<uint32_t>(ADDR_TYPE::management), static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(MSG_TYPE::trigger));
                req->set_object_management(this->client_ctx->management);
                req->set_object_cluster(this->client_ctx->computation);
                req->set_token(this->client_ctx->token);
                req->set_taskid(taskid);
                Workflow::start_series_work(task, [&group](const SeriesWork *) { group.done(); });
                group.wait();
                return 0;
            }
        }
    }
}
