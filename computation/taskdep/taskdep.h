#pragma once
#include <string>
#include "proto/taskdep/message.h"
#include <misc/adapter/noncopy.h>
#include <vector>
#include "computation/infer/infer.h"

namespace jxtd
{
    namespace computation
    {
        namespace taskdep
        {
            using Endpoint = ::jxtd::proto::taskdep::Endpoint;
            using TaskdepServer = struct TaskdepServer;
            using TaskdepSetupContext = struct TaskdepSetupContext;
            using Taskmanager = ::jxtd::computation::infer::Taskmanager;
            using Servermanager = ::jxtd::computation::infer::DrccpServerManager;

            struct TaskdepSetupContext
            {
                Endpoint endpoint;
                bool ssl;
                int family;
                std::string ca;
                std::string cert;
                std::string key;
                int response_timeout = -1;
                int request_size_limit = 200 * 1024 * 1024;
            };

            class ClusterServer: private ::jxtd::misc::adapter::NonCopy
            {
            public:
                ClusterServer();
                ~ClusterServer();
                int setup(const TaskdepSetupContext &ctx);
                int start(std::string name, Taskmanager &t_manager, Servermanager &s_manager);
                void stop();

            private:
                TaskdepServer *server_ctx;
            };
        }
    }
}