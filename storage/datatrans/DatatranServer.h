#pragma once
#include <string>
#include <mutex>
#include <storage/datamanagement/datamanagement.h>

namespace jxtd{
    namespace storage{
        namespace datatransfer{
            using Datamanager = ::jxtd::storage::datamanagement::Datamanager;
            //配置信息
            struct DatatranServerSetupContext{
                int family;
                std::string addr;
                int port;
                bool ssl;
                std::string ca_path;
                std::string key_path;
                std::string cert_path;
            };
            //class jxtd::storage::datatrans::DatatranServer
            struct DatatranServerContext;
            class DatatranServer{
            public:
                DatatranServer();
                ~DatatranServer();
                explicit DatatranServer(const DatatranServerSetupContext& ctx);
                void setup_context(const DatatranServerSetupContext& ctx);
                int start(Datamanager& adm, std::mutex& mtx);
                int stop();
            private:
                DatatranServerContext* ctx_;
            };
        }
    }
}
