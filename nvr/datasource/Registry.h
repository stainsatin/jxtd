#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <cstdint>

namespace jxtd{
    namespace nvr{
        namespace datasource{
            class Registry{
            public:
                enum action{
                    Register = 0,
                    Unregister,
                    Offline,
                    Online
                };
                enum proto{
                    Http = 0,
                    Https,
                    ssh,
                    jxtd,
                    tcp
                };
                //management config
                void load_config(const std::string& config_name, const std::string& url, bool ssl, uint32_t port);
                //datasource config
                uint32_t register_datasource(const std::string& dtname, const std::string& url, uint32_t proto, uint32_t port, const std::string& config_name);
            private:
                std::unordered_map<std::string, std::tuple<std::string, bool, uint32_t>> configMap;
            };
        }
    }
}
