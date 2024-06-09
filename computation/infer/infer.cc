#include "SubmitData.pb.h"
#include <misc/adapter/noncopy.h>
#include "infer.h"
#include <computation/drccp/server.h>
#include <utility>
#include <workflow/WFFacilities.h>
#include <computation/datatransfer/client_task.h>
#include <computation/engine/Engine.h>
#include <dlfcn.h>
#include <filesystem>
#include "workflow/HttpMessage.h"
#include "workflow/WFTaskFactory.h"

namespace jxtd
{
    namespace computation
    {
        namespace infer
        {
            using DataClient = ::jxtd::computation::datatransfer::DataClient;
            using DataRequestContext = ::jxtd::computation::datatransfer::DataRequestContext;
            using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;

            static std::string storage_url = "";
            static int storage_port = 0;
            static bool storage_ssl = false;

            void set_storage_datatransfer(const std::string &url, int port, bool ssl)
            {
                storage_url = url;
                storage_port = port;
                storage_ssl = ssl;
            }

            SubmitRequest::SubmitRequest(const SubmitRequest &other)
            {
                SubmitRequestData.CopyFrom(other.SubmitRequestData);
            }

            SubmitRequest::SubmitRequest(SubmitRequest &&other) noexcept
            {
                SubmitRequestData.Swap(&(other.SubmitRequestData));
            }

            SubmitRequest &SubmitRequest::operator=(const SubmitRequest &other)
            {
                SubmitRequestData.CopyFrom(other.SubmitRequestData);
                return *this;
            }

            SubmitRequest &SubmitRequest::operator=(SubmitRequest &&other) noexcept
            {
                if (this != &other)
                {
                    SubmitRequestData.Swap(&(other.SubmitRequestData));
                }
                return *this;
            }

            int SubmitRequest::set_taskid(uint32_t taskid)
            {
                SubmitRequestData.set_taskid(taskid);
                return 0;
            }

            uint32_t SubmitRequest::get_taskid() const
            {
                return SubmitRequestData.taskid();
            }

            int SubmitRequest::set_input(const std::string &url, const std::string &data)
            {
                if(!data.empty())
                    SubmitRequestData.set_input_data(data);
                if(!url.empty())
                    SubmitRequestData.set_input_path(url);
                return 0;
            }

            std::string SubmitRequest::get_input_path() const
            {
                return SubmitRequestData.input_path();
            }

            std::string SubmitRequest::get_input_data() const
            {
                return SubmitRequestData.input_data();
            }

            int SubmitRequest::set_output(const std::string &url, uint32_t directory_policy, uint32_t empty_policy)
            {
                SubmitRequestData.set_directory_policy(directory_policy);
                SubmitRequestData.set_empty_policy(empty_policy);
                SubmitRequestData.set_output_path(url);
                return 0;
            }

            std::string SubmitRequest::get_output_path() const
            {
                return SubmitRequestData.output_path();
            }

            uint32_t SubmitRequest::get_directory_policy() const
            {
                return SubmitRequestData.directory_policy();
            }

            uint32_t SubmitRequest::get_empty_policy() const
            {
                return SubmitRequestData.empty_policy();
            }

            int SubmitRequest::set_period(uint32_t times)
            {
                SubmitRequestData.set_period(times);
                return 0;
            }

            uint32_t SubmitRequest::get_period() const
            {
                return SubmitRequestData.period();
            }

            std::string SubmitRequest::dump() const
            {
                std::string binary;
                SubmitRequestData.SerializeToString(&binary);
                return binary;
            }

            int SubmitRequest::parse(const std::string &binary)
            {
                SubmitRequestData.ParseFromString(binary);
                return 0;
            }

            SubmitResponse::SubmitResponse(const SubmitResponse &other)
            {
                SubmitResponseData.CopyFrom(other.SubmitResponseData);
            }

            SubmitResponse::SubmitResponse(SubmitResponse &&other) noexcept
            {
                SubmitResponseData.Swap(&(other.SubmitResponseData));
            }

            SubmitResponse &SubmitResponse::operator=(const SubmitResponse &other)
            {
                SubmitResponseData.CopyFrom(other.SubmitResponseData);
                return *this;
            }

            SubmitResponse &SubmitResponse::operator=(SubmitResponse &&other) noexcept
            {
                if (this != &other)
                {
                    SubmitResponseData.Swap(&(other.SubmitResponseData));
                }
                return *this;
            }

            int SubmitResponse::set_taskid(uint32_t taskid)
            {
                SubmitResponseData.set_taskid(taskid);
                return 0;
            }

            uint32_t SubmitResponse::get_taskid() const
            {
                return SubmitResponseData.taskid();
            }

            int SubmitResponse::set_times(uint32_t times)
            {
                SubmitResponseData.set_times(times);
                return 0;
            }

            uint32_t SubmitResponse::get_times() const
            {
                return SubmitResponseData.times();
            }

            int SubmitResponse::set_result(const std::string &result)
            {
                SubmitResponseData.set_result(result);
                return 0;
            }

            std::string SubmitResponse::get_result() const
            {
                return SubmitResponseData.result();
            }

            int SubmitResponse::set_output(const std::string &output_path, uint32_t directory_policy, uint32_t empty_policy)
            {
                SubmitResponseData.set_output_path(output_path);
                SubmitResponseData.set_directory_policy(directory_policy);
                SubmitResponseData.set_empty_policy(empty_policy);
                return 0;
            }

            std::string SubmitResponse::get_output_path() const
            {
                return SubmitResponseData.output_path();
            }

            uint32_t SubmitResponse::get_directory_policy() const
            {
                return SubmitResponseData.directory_policy();
            }

            uint32_t SubmitResponse::get_empty_policy() const
            {
                return SubmitResponseData.empty_policy();
            }

            std::string SubmitResponse::dump() const
            {
                std::string binary;
                SubmitResponseData.SerializeToString(&binary);
                return binary;
            }

            int SubmitResponse::parse(const std::string &binary)
            {
                if (SubmitResponseData.ParseFromString(binary) != 1)
                    return -1;
                return 0;
            }

            static void drccp_on_server_receive_result(const std::string &token, const std::string &url, int port, bool ssl, const std::string &result)
            {
                SubmitResponse resp;
                resp.parse(result);
                std::string outputpath = resp.get_output_path();
                uint32_t directory_policy = resp.get_directory_policy();
                uint32_t empty_policy = resp.get_empty_policy();
                if(!outputpath.empty())
                {
                    SubTask *task = nullptr;
                    size_t pos = outputpath.find_first_of(":");
                    std::string output_proto = outputpath.substr(0, pos);
                    if(output_proto == "jxtd")
                    {
                        DataClient client;
                        client.setup({url, port, ssl});

                        DataRequestContext context;
                        context.src = "jxtd://" + url + ":" + std::to_string(port);
                        context.dest = outputpath;
                        context.direction = 0x1;
                        context.token = token;

                        outputpath.erase(0, pos + 3);
                        pos = outputpath.find_first_of(":");
                        std::string output_url = outputpath.substr(0, pos);
                        outputpath.erase(0, pos + 1);
                        pos = outputpath.find_first_of("/");
                        std::string output_path;
                        if(pos > 0)
                            output_path = outputpath.substr(pos + 1);
                        int output_port = std::stoi(outputpath);

                        Adtsrc temp;
                        temp.taskid = resp.get_taskid();
                        temp.type = 0x1;
                        temp.binary = resp.get_result();
                        context.adtsrcs.push_back(temp);

                        std::vector<std::pair<std::string, std::string>> null_mid_name;
                        std::vector<std::string> null_binary;

                        task = client.create({output_url, output_port, ssl}, context, null_binary, null_mid_name);
                    }
                    else if(output_proto == "http" || output_proto == "https")
                    {
                        auto task_ = WFTaskFactory::create_http_task(outputpath, 5, 2, nullptr);
                        protocol::HttpRequest *req = task_->get_req();
                        req->set_request_uri(output_proto + "://" + url + ":" + std::to_string(port) + "/");
                        req->set_method("PUT");
                        req->append_output_body(resp.get_result().data(), resp.get_result().size());
                        task = task_;
                    }
                    if(task != nullptr)
                    {
                        WFFacilities::WaitGroup group(1);
                        Workflow::start_series_work(task, [&group](const SeriesWork*){group.done();});
                        group.wait();
                    }
                }
            }

            using Tensor = ::jxtd::computation::infer::Tensor;
            using Engine = ::jxtd::computation::engine::jxtd_Engine;

            static std::string drccp_on_server_receive_payload(const std::string &token, const std::string &url, int port, bool ssl, const std::string &binary)
            {
                SubmitRequest req;
                SubmitResponse resp;
                req.parse(binary);
                resp.set_taskid(req.get_taskid());
                resp.set_times(req.get_period());
                Tensor *input = new Tensor;
                std::string inputpath = req.get_input_path();
                DataClient client;
                DataRequestContext context;
                context.dest = inputpath;
                context.src = "jxtd://" + url + ":" + std::to_string(port);
                context.direction = 0x0;
                context.token = token;
                client.setup({url, port, ssl});
                if(inputpath.empty())
                    input->set_data(req.get_input_data().data(), req.get_input_data().size());
                else
                {
                    SubTask *task = nullptr;
                    size_t pos = inputpath.find_first_of(":");
                    std::string input_proto = inputpath.substr(0, pos);
                    if(input_proto == "jxtd")
                    {
                        std::vector<std::string> inputdata;
                        inputpath.erase(0, pos + 3);
                        size_t position = inputpath.find_first_of(":");
                        std::string dest_url = inputpath.substr(0, position);
                        inputpath.erase(0, position + 1);
                        position = inputpath.find_first_of("/");
                        std::string dest_path;
                        if(position > 0)
                            dest_path = inputpath.substr(position + 1);
                        std::vector<std::pair<std::string, std::string>> null_mid_name;
                        task = client.create({dest_url, std::stoi(inputpath), ssl}, context, std::ref(inputdata), null_mid_name);
                        input->set_data(inputdata.front().data(), inputdata.front().size());
                    }
                    else if(input_proto == "http" || input_proto == "https")
                    {
                       
                    }
                    else
                        return "";
                    if(task != nullptr)
                    {
                        WFFacilities::WaitGroup group(1);
                        Workflow::start_series_work(task, [&group](const SeriesWork*){group.done();});
                        group.wait();
                    }
                }
                
                std::vector<std::pair<std::string, std::string>> mid_name;
                {
                    Adtsrc temp;
                    temp.taskid = req.get_taskid();
                    temp.type = 0x0; // 获取engine
                    context.adtsrcs.push_back(temp);
                    std::vector<std::string> null_binary;
                    auto task = client.create({storage_url, storage_port, storage_ssl}, context, null_binary, mid_name);
                    WFFacilities::WaitGroup group(1);
                    Workflow::start_series_work(task, [&group](const SeriesWork*){group.done();});
                    group.wait();
                }
                // 目前对mid_name应该只有一个，因此这里只写了一个
                Tensor *output = new Tensor;
                if(mid_name.size() == 1)
                {
                    Engine *engine = nullptr;
                
                    auto handle = dlopen(("../Task_" + std::to_string(req.get_taskid()) + "/JXTDENGINE_" + mid_name.front().first + "_" + mid_name.front().second + ".so").c_str(), RTLD_LAZY);
                    if(handle != nullptr)
                    {
                        typedef Engine *(*generate)();
                        generate generate_parent = reinterpret_cast<generate>(dlsym(handle, ("generate_" + mid_name.front().first + "_" + mid_name.front().second).c_str()));
                        if(generate_parent != nullptr)
                            engine = generate_parent();
                    }

                    if(engine != nullptr)
                    {
                        engine->init_env(); // 这三个操作没有检查失败
                        engine->compute(input, output);
                        engine->post_env();
                        delete engine;
                    }
                    dlclose(handle);
                    std::filesystem::remove_all("computation/Task_" + std::to_string(req.get_taskid()));
                }

                delete input;
                if(output->get_data_nocopy() != nullptr)
                    resp.set_result(static_cast<const char *>(output->get_data_nocopy()));
                resp.set_output(req.get_output_path(), req.get_directory_policy(), req.get_empty_policy());
                delete output;
                return std::move(resp.dump());
            }

            DrccpServerManager::~DrccpServerManager()
            {
                for (auto &server : servers)
                {
                    delete server.second;
                }
                servers.clear();
            }

            void DrccpServerManager::load_server_config(const std::string &token, const std::string &url, int port, bool ssl, const std::string &name, const jxtd::computation::drccp::DrccpServer::DrccpSetupContext &setup, const jxtd::computation::drccp::DrccpServer::DrccpProxyOriginContext &proxy)
            {
                Server *load_server = new Server;
                load_server->server.setup(setup);
                load_server->SetupContext = setup;
                load_server->OriginContext = proxy;
                load_server->server.register_receive_result(std::bind(drccp_on_server_receive_result, token, url, port, ssl, std::placeholders::_1));
                load_server->server.register_receive_payload(std::bind(drccp_on_server_receive_payload, token, url, port, ssl, std::placeholders::_1));
                servers.insert(std::make_pair(name, load_server));
            }

            int DrccpServerManager::start(const std::string &name)
            {
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                {
                    return -1;
                }
                return target_server->second->server.start();
            }

            int DrccpServerManager::stop(const std::string &name)
            {
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                {
                    return -1;
                }
                return target_server->second->server.stop();
            }

            int DrccpServerManager::signin(const std::string &name, const std::string &cluster_tag, const std::string &boot_addr, int port)
            {
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                {
                    return -1;
                }
                return target_server->second->server.signin(cluster_tag, boot_addr, port);
            }

            int DrccpServerManager::signin(const std::string &name, const std::string &tag, const std::string &peer)
            {
                std::map<std::string, Server *>::iterator connect_server = servers.find(peer);
                if (connect_server == servers.end())
                {
                    return -1;
                }
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                {
                    return -1;
                }
                return target_server->second->server.signin(tag, connect_server->second->SetupContext.listen_addr, connect_server->second->SetupContext.port);
            }

            int DrccpServerManager::logout(const std::string &name)
            {
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                {
                    return -1;
                }
                return target_server->second->server.logout();
            }

            using DrccpProxyOriginContext = ::jxtd::computation::drccp::DrccpServer::DrccpProxyOriginContext;
            SubTask *DrccpServerManager::submit(const std::string &name, const SubmitRequest &req, Taskmanager &manager)
            {
                std::map<std::string, Server *>::iterator target_server = servers.find(name);
                if (target_server == servers.end())
                    return nullptr;
                auto on_client_recieve_result = [&manager](const std::string &result_)
                {
                    if(result_.empty())
                        return;
                    SubmitResponse response;
                    response.parse(result_);
                    // std::string result = response.get_result();
                    manager.remove_task(response.get_taskid(), true); // 执行删除
                };
                target_server->second->server.register_client_receive_result(on_client_recieve_result);
                auto submit_ = [target_server](std::string payload, DrccpProxyOriginContext ctx)
                {
                    target_server->second->server.submit(payload, ctx);
                };
                auto task = WFTaskFactory::create_go_task(name, submit_, req.dump(), target_server->second->OriginContext);
                return task;
            }

            Taskmanager::Taskmanager()
            {
                this->client = nullptr;
                this->server = nullptr;
            }

            Taskmanager::~Taskmanager()
            {
                if(this->client != nullptr)
                    delete this->client;
                if(this->server != nullptr)
                    delete this->server;
            }

            int Taskmanager::deploy_task(uint64_t taskid, const std::string &input_path, const std::string &input_data, const std::string &output_path, uint32_t directorypolicy, uint32_t emptypolicy, uint32_t period, uint32_t resultpolicy, uint32_t triggerpolicy, std::vector<uint32_t> backupid)
            {
                std::unique_lock<std::mutex> lock(this->task_mutex);
                if(this->tasks.find(taskid) != this->tasks.end())
                    return -1;
                // 没有检查重复，应该不用
                Taskinfo taskinfo = {static_cast<uint64_t>(this->tasks.size()), input_path, input_data, output_path, directorypolicy, emptypolicy, period, backupid, resultpolicy, triggerpolicy};
                this->tasks.emplace(taskid, taskinfo);

                for(auto &single_edge : this->edges)
                {
                    single_edge.push_back(false);
                }

                edges.push_back({});
                edges.back().resize(this->tasks.size(), false);
                return 0;
            }

            int Taskmanager::add_deps(uint64_t taskid, std::vector<uint64_t> deps)
            {
                std::unique_lock<std::mutex> lock(this->task_mutex);
                auto iter = this->tasks.find(taskid);
                if(iter == this->tasks.end())
                    return -1;
                for(const auto dep : deps)
                {
                    auto it = this->tasks.find(dep);
                    if(it == this->tasks.end())
                        return -1;
                    edges.at(iter->second.dep_pos).at(it->second.dep_pos) = true;
                }
                return 0;
            }

            const Taskmanager::Taskinfo *Taskmanager::get_task(uint64_t taskid)
            {
                auto iter = this->tasks.find(taskid);
                if(iter == this->tasks.end())
                    return nullptr;
                return &(iter->second);
            }

            SubTask *Taskmanager::submit(DrccpServerManager &manager, const std::string &name, uint64_t taskid)
            {
                std::unique_lock<std::mutex> lock(this->task_mutex);
                auto iter = this->tasks.find(taskid);
                if(iter == this->tasks.end())
                    return nullptr;
                for(const auto &edge : (edges.at(iter->second.dep_pos)))
                    if(edge)
                        return nullptr;
                const auto &taskinfo = iter->second;
                ::jxtd::computation::infer::SubmitRequest req;
                req.set_taskid(taskid);
                req.set_input(taskinfo.input_path, taskinfo.input_data);
                req.set_output(taskinfo.output_path, taskinfo.directory_policy, taskinfo.empty_policy);
                req.set_period(taskinfo.period); // 目前仅仅设置以下period，以后进行修改
                return manager.submit(name, req, *this);
            }

            int Taskmanager::set_src(const Endpoint &endpoint, bool ssl, const std::string &token)
            {
                if(this->client == nullptr)
                    this->client = new ClusterClient;
                this->client->endpoint = endpoint;
                this->client->ssl = ssl;
                this->client->token = token;
                return 0;
            }

            int Taskmanager::set_dest(const Endpoint &storage)
            {
                if(this->server == nullptr)
                    this->server = new Endpoint;
                *(this->server) = storage;
                return 0;
            }

            using Request = ::jxtd::proto::taskdep::Request;
            using Response = ::jxtd::proto::taskdep::Response;
            using TaskdepTaskFactory = WFNetworkTaskFactory<Request, Response>;
            using MSG_TYPE = ::jxtd::proto::taskdep::MSG_TYPE;
            using ADDR_TYPE = ::jxtd::proto::taskdep::ADDR_TYPE;

            void Taskmanager::remove_task(uint64_t taskid, bool send)
            {
                {
                    std::unique_lock<std::mutex> lock(this->task_mutex);
                    auto iter = this->tasks.find(taskid);
                    if(iter == this->tasks.end())
                        return;
                    auto dep_pos = iter->second.dep_pos;
                    edges.erase(edges.begin() + dep_pos);
                    tasks.erase(iter);
                
                    for(auto &single_edge : edges)
                        single_edge.erase(single_edge.begin() + dep_pos);
                }

                if(send)
                {
                    if(this->client == nullptr || this->server == nullptr || !this->client->endpoint.check() || !this->server->check())
                        return;
                    auto task = TaskdepTaskFactory::create_client_task (
                        this->client->ssl ? TT_TCP_SSL : TT_TCP,
                        server->get_url(),
                        server->get_port(),
                        1,
                        nullptr // 这里仅仅发送，并没有加入发送done后是否发送成功的处理
                    );
                    auto req = task->get_req();
                    req->set_type(static_cast<uint32_t>(ADDR_TYPE::cluster), static_cast<uint32_t>(ADDR_TYPE::storage), static_cast<uint32_t>(MSG_TYPE::done));
                    req->set_object_cluster(this->client->endpoint);
                    req->set_object_storage(*this->server);
                    req->set_token(this->client->token);
                    req->set_taskid(taskid);
                    WFFacilities::WaitGroup group(1);
                    auto series = Workflow::create_series_work(task, [&group](const SeriesWork* series){
                        group.done();
                    });
                    series->start();
                    group.wait();
                }
            }
        }
    }
}
