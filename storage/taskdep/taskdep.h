#pragma once
#include <string>
#include "proto/taskdep/message.h"
#include <misc/adapter/noncopy.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "storage/datamanagement/datamanagement.h"

namespace jxtd
{
    namespace storage
    {
        namespace taskdep
        {
            using Endpoint = ::jxtd::proto::taskdep::Endpoint;
            using TaskdepServer = struct TaskdepServer;
            using TaskdepSetupContext = struct TaskdepSetupContext;
            using Engine = ::jxtd::proto::taskdep::Request::Engine;
            using Extfile = Engine::Extfile;
            using Datamanager = ::jxtd::storage::datamanagement::Datamanager;

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

            class StorageServer: private ::jxtd::misc::adapter::NonCopy
            {
            public:
                StorageServer();
                ~StorageServer();
                int setup(const TaskdepSetupContext &ctx);
                int start(Datamanager &d_manager, std::mutex &task_mutex);
                void stop();

            private:
                TaskdepServer *server_ctx;
            };
        }
    }
}