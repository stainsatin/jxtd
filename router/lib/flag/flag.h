#pragma once
#include <string>

namespace jxtd
{
    namespace router
    {
        namespace flag
        {
            struct Flag
            {
                // 这四个包括proto，url，port，path
                std::string management;
                std::string computation;
                std::string storage;
                // 这两个只有url+port
                std::string datatransfer_router;
                std::string datatransfer_storage;
                bool ssl;
            
                int port = 2878;
                std::string ipv4 = "192.168.28.1";
                std::string ipv6 = "[fe80::2878::1]";
            };
            using Flag = struct Flag;
        }
    }
}    
