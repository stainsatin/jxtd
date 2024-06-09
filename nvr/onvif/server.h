#pragma once
#include <cstdint>
#include <string>
#include <misc/adapter/noncopy.h>
namespace jxtd{
    namespace nvr{
        namespace onvif{
            class OnvifAdapter;
            using OnvifForwardServerContext = struct OnvifForwardServerContext;
            struct OnvifForwardServerSetupContext{
                uint32_t uid;
                int family;
                uint32_t port;
                bool ssl;
                std::string ca;
                std::string cert;
                std::string key;
                OnvifAdapter* adapter;
            };
            class OnvifForwardServer:public ::jxtd::misc::adapter::NonCopy{
                public:
                    explicit OnvifForwardServer(const OnvifForwardServerSetupContext& setup);
                    ~OnvifForwardServer();
                    int start();
                    int stop();
                protected:
                    OnvifForwardServer();
                private:
                    OnvifForwardServerContext* server_ctx;
            };
        }
    }
}
