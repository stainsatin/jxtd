#include "client_task.h"

#include "workflow/WFTaskFactory.h"
#include <fstream>
#include <filesystem>
#include "misc/protocol/protocol.h"

namespace jxtd
{
    namespace computation
    {
        namespace datatransfer
        {
            using DataRequest = ::jxtd::proto::datatransfer::Request;
            using DataResponse = ::jxtd::proto::datatransfer::Response;

            DataClient::DataClient()
            {
                this->client_ctx = new DataClientContext;
            }

            DataClient::~DataClient()
            {
                if (this->client_ctx)
                    delete this->client_ctx;
                this->client_ctx = nullptr;
            }

            using DataTask = WFNetworkTask<DataRequest, DataResponse>;
            using DataTaskFactory = WFNetworkTaskFactory<DataRequest, DataResponse>;

            static void call_back_t(std::vector<std::string> &input, std::vector<std::pair<std::string, std::string>> &master_mid_name, DataTask *task);
            static void resp_dispose(std::vector<std::pair<std::string, std::string>> &master_mid_name, const ::jxtd::proto::datatransfer::Dtsrc &dtsrc);
            static void deps_dispose(std::ofstream &outfile, const std::vector<std::pair<Adtsrc, std::string>> &deps, const std::string &filepath);

            static void call_back_t(std::vector<std::string> &input, std::vector<std::pair<std::string, std::string>> &master_mid_name, DataTask *task)
            {
                if (task->get_state() != WFT_STATE_SUCCESS)
                    return;
                auto resp = task->get_resp();
                for(const auto &dtsrc : resp->get_dtsrcs())
                {
                    if(dtsrc.master.type == 0x0) // engine
                        resp_dispose(master_mid_name, dtsrc);
                    else if(dtsrc.master.type == 0x2) // binary
                        input.emplace_back(dtsrc.master.binary);
                }
            }

            static void resp_dispose(std::vector<std::pair<std::string, std::string>> &master_mid_name, const ::jxtd::proto::datatransfer::Dtsrc &dtsrc)
            {
                std::ofstream outfile;
                const auto &master_ = dtsrc.master;

                if(master_.engineattr.first.empty() || master_.engineattr.second.empty() || master_.binary.empty())
                    return;
                master_mid_name.emplace_back(master_.engineattr.first, master_.engineattr.second);
                std::string filepath = "../Task_" + std::to_string(master_.taskid) + "/";
                std::filesystem::create_directories(filepath);
                outfile.open((filepath + "JXTDENGINE_" + master_.engineattr.first + "_" + master_.engineattr.second + ".so").c_str(), std::ios::binary | std::ios::out);
                if (!outfile.is_open())
                    return;
                outfile.write(master_.binary.data(), master_.binary.size());
                outfile.close();
                deps_dispose(outfile, dtsrc.deps, filepath);
            }

            static void deps_dispose(std::ofstream &outfile, const std::vector<std::pair<Adtsrc, std::string>> &deps, const std::string &filepath)
            {
                for (const auto &dep : deps)
                {
                    const auto &dep_master = dep.first;

                    std::string path = dep.second;
                    size_t pos = 0;
                    if((pos = path.find_last_of("/")) != std::string::npos)
                    {
                        path.erase(pos, path.size());
                    }
                    std::filesystem::create_directories(filepath + path);

                    outfile.open((filepath + dep.second).c_str(), std::ios::out | std::ios::binary);
                    if (!outfile.is_open())
                        continue;
                    outfile.write(dep_master.binary.data(), dep_master.binary.size());
                    outfile.close();
                }
            }

            void DataClient::setup(const DataClientSetupContext &ctx)
            {
                this->client_ctx->addr = ctx.addr;
                this->client_ctx->ssl = ctx.ssl;
                this->client_ctx->port = ctx.port;
            }

            SubTask* DataClient::create(const DataServerContext &ctx, const DataRequestContext &req_ctx, std::vector<std::string> &input, std::vector<std::pair<std::string, std::string>> &master_mid_name)
            {
                auto task = DataTaskFactory::create_client_task(
                    this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                    ctx.addr,
                    ctx.port,
                    1,
                    [&input, &master_mid_name](DataTask *task)
                    {
                        call_back_t(input, master_mid_name, task);
                    });
                auto req = task->get_req();
                req->set_token(req_ctx.token);
                req->set_dest(req_ctx.dest);
                req->set_src(req_ctx.src);
                req->set_direction(req_ctx.direction);
                for(const auto &adtsrc : req_ctx.adtsrcs)
                {
                    req->add_adtsrc(adtsrc);
                }
                return task;
            }

            const DataClientContext *DataClient::get_client()
            {
                return client_ctx;
            }
        }
    }
}
