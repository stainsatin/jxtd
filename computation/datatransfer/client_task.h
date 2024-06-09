#pragma once
#include <string>
#include <functional>
#include <cstdint>
#include <vector>
#include "proto/datatransfer/message.h"
#include <workflow/Workflow.h>

namespace jxtd
{
    namespace computation
    {
        namespace datatransfer
        {
            using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;
            struct DataRequestContext
            {
                std::string src;
                std::string dest;
                std::string token;
                uint32_t direction; // 0x0---pull, 0x1---push, 其他的值返回-1
                // 按照先taskid，再engine，再endpoint，最后binary的顺序，如若修改顺序，请在create中自行修改
                std::vector<Adtsrc> adtsrcs;
            };
            using DataRequestContext = struct DataRequestContext;

            struct DataSideContext
            {
                std::string addr;
                int port;
                bool ssl;
            };
            using DataServerContext = struct DataSideContext;      // 服务端
            using DataClientContext = struct DataSideContext;      // 客户端
            using DataClientSetupContext = struct DataSideContext; // 客户端建立上下文

            class DataClient
            {
            public:
                DataClient();
                ~DataClient();

                void setup(const DataClientSetupContext &ctx);                               // set up client
                SubTask* create(const DataServerContext &ctx, const DataRequestContext &req_ctx, std::vector<std::string> &input, std::vector<std::pair<std::string, std::string>> &master_mid_name); // create ready-for-start task
                const DataClientContext *get_client();                                       // get client

            private:
                DataClientContext *client_ctx;
            };
        }
    }
}
