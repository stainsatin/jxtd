#pragma once
#include <string>
#include <functional>
#include "proto/taskdep/message.h"

namespace jxtd
{
    namespace management
    {
        namespace taskdep
        {
            using Endpoint = ::jxtd::proto::taskdep::Endpoint;
            using Request = ::jxtd::proto::taskdep::Request;
            using Response = ::jxtd::proto::taskdep::Response;
            using TaskdepClientContext = struct TaskdepClientContext;
            using Engine = Request::Engine;
            using Datasource = Request::Datasource;
            using Attr = Request::Attr;

            struct TaskdepClientContext
            {
                Endpoint management;
                Endpoint computation;
                Endpoint storage;
                std::string token;
                bool ssl;
            };

            class ManagementClient
            {
            public:
                ManagementClient();
                ~ManagementClient();
                int setup(const TaskdepClientContext &ctx);
                // int create_loopback_task();
                uint64_t create_neogiate_task(uint64_t taskid, const Engine &engine);
                int create_deploy_task(uint64_t taskid, const Datasource &datasource, const Attr &attr, int busy_retry_max);
                int create_terminate_task(uint64_t taskid);
                int create_trigger_task(uint64_t taskid);

            private:
                TaskdepClientContext *client_ctx;
            };
        }
    }
}