#pragma once
#include <string>
#include <functional>
#include <misc/adapter/noncopy.h>
namespace jxtd{
    namespace computation{
        namespace drccp{
            using DrccpServerContext = struct DrccpServerContext;
            using DrccpClientContext = struct DrccpClientContext;
            class DrccpServer:private ::jxtd::misc::adapter::NonCopy{
                public:
                    using DrccpSetupContext = struct{
                        std::string listen_addr;
                        int family;
                        int port;
                        bool ssl;
                        std::string ca;
                        std::string cert;
                        std::string key;
                    };
                    using DrccpProxyOriginContext = struct{
                        std::string origin_addr;
                        int origin_port;
                    };
                    using on_server_receive_payload_t = std::function<std::string (const std::string&)>;
                    using on_server_receive_result_t = std::function<void (const std::string&)>;
                    using on_client_receive_result_t = std::function<void (const std::string&)>;
                    DrccpServer();
                    ~DrccpServer();
                    int setup(const DrccpSetupContext& ctx);
                    int start();
                    int stop();
                    int signin(const std::string& cluster_tag, const std::string& boot_addr, int boot_port);
                    int logout();
                    int submit(const std::string& payload, const DrccpProxyOriginContext& ctx);
                    void remove_receive_payload();
                    void register_receive_payload(on_server_receive_payload_t callback);
                    void remove_receive_result();
                    void register_receive_result(on_server_receive_result_t callback);
                    void remove_client_receive_result();
                    void register_client_receive_result(on_client_receive_result_t callback);
                private:
                    DrccpServerContext* server_ctx;
                    DrccpClientContext* client_ctx;
            };
        }
    }
}
