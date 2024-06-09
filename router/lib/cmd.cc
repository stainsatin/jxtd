#include "misc/parser/at/ATParser.h"
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "cmd.h"
#include <fstream>
#include <workflow/Workflow.h>
#include <functional>
#include <mutex>
#include "proto/datatransfer/message.h"
#include "workflow/HttpMessage.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"
#include <random>

namespace jxtd
{
    namespace router
    {
        static std::vector<std::pair<std::string, std::string>> engine_private;
        static std::vector<std::pair<std::string, std::string>> engine_cloud;
        static std::string user;
        Taskresult task;
        Taskmanager manager;
        Taskdepmanagement management;
        Flag flag_;

        static void process(WFHttpTask *task, Taskresult *result)
        {
            protocol::HttpRequest *req = task->get_req();
            size_t len;
            if(result->get_taskid() != 0)
            {
                const void *body;
                req->get_parsed_body(&(body), &len);
                result->get_data_set() = {static_cast<const char *>(body), len};
                if(len == 0)
                    result->set_status(false);
                result->notify();
            }
        }

        Taskresult::Taskresult()
        {
            this->server = nullptr;
            this->taskid = 0;
            this->status = true;
        }

        Taskresult::~Taskresult()
        {
            if(this->server != nullptr)
            {
                this->server->stop();
                delete this->server;
            }
        }

        void Taskresult::set_status(bool status)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->status = status;
        }

        bool Taskresult::get_status()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            return this->status;
        }

        int Taskresult::start()
        {
            if(this->server != nullptr)
                return -1;
            this->server = new WFHttpServer(std::bind(process, std::placeholders::_1, this));
            return this->server->start(AF_INET, "localhost", 2222);
        }

        int Taskresult::stop()
        {
            if(this->server != nullptr)
            {
                this->server->stop();
                delete this->server;
                this->server = nullptr;
            }
            return 0;
        }

        std::string Taskresult::get_data()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            return this->data;
        }

        std::string &Taskresult::get_data_set()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            return this->data;
        }

        void Taskresult::remove()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->taskid = 0;
            this->status = true;
            this->data.clear();
        }

        void Taskresult::wait()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->condition.wait(lock, [this](){
                return (this->taskid == 0 || !this->status || !this->data.empty());
            });
        }

        void Taskresult::notify()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->condition.notify_one();
        }

        void Taskresult::set(uint64_t taskid)
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            this->taskid = taskid;
            this->data.clear();
            this->status = true;
        }

        uint64_t Taskresult::get_taskid()
        {
            std::unique_lock<std::mutex> lock(this->mutex_);
            return this->taskid;
        }

        using ATMessage = ::jxtd::misc::parser::at::ATMessage;
        using Type = ATMessage_Type;

        std::string REG(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::CMD:
                {
                    if(message.param.size() != 1)
                        return "AT+ENVAL";
                    // 仅设置第一个名字
                    std::string name = message.param.front();
                    if(user == name)
                        return "AT+ERR=0,name is registered";
                    // 如果已经注册，就重新注册
                    if(!user.empty())
                        std::filesystem::remove_all("../" + user + "/");
                    user = message.param.front();
                    std::filesystem::create_directory("../" + user + "/");
                    return "AT+OK";
                }
                case Type::TEST:
                {
                    if(user.empty())
                        return "AT+ERR=-2,name is not registered";
                    return "AT+OK";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string RTUNREG(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::CMD:
                {
                    if(message.param.size() != 0)
                        return "AT+ENVAL";
                    if(!user.empty())
                    {
                        std::filesystem::remove_all("../" + user + "/");
                        user.clear();
                        return "AT+OK";
                    }
                    return "AT+ERR";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string RTBUSY(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::TEST:
                {
                    if(message.param.size() != 0)
                        return "AT+ENVAL";
                    if(task.get_taskid() != 0)
                        return "AT+BUSY";
                    return "AT+IDLE";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        static std::string create_id()
        {
            std::string id;
            srand(time(NULL));
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> distribution(0, 9);
            for(int i = 0; i < 4; i++)
                id += '0' + distribution(gen);
            std::uniform_int_distribution<int> distribution_(0, 51);
            for(int i = 0; i < 4; i++)
            {
                int randomNumber = distribution_(gen);
                if (randomNumber < 26) {
                    id += 'A' + randomNumber;
                } else {
                    id += 'a' + randomNumber - 26;
                }
            }
            auto check = [](const std::string &id){
                for(const auto &engine : engine_cloud)
                    if(engine.second == id)
                        return false;
                for(const auto &engine : engine_private)
                    if(engine.second == id)
                        return false;
                return true;
            };
            do
            {
                std::shuffle(id.begin(), id.end(), gen);
            }
            while(!check(id));
            return id;
        }

        std::string MDDECL(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::CMD:
                {
                    if(user.empty())
                        return "AT+MDINFO=-4,the service is not registered";
                    if(message.param.size() == 2)
                    {
                        // name和mid的情况，如果name相同则不可行，以name为基准
                        for(const auto &engine : engine_cloud)
                        {
                            if(engine.first == message.param[0] || engine.second == message.param[1])
                                return "AT+MDINFO=0," + engine.first + "," + engine.second + ",remote model has existed";
                        }
                        for(const auto &engine : engine_private)
                        {
                            if(engine.first == message.param[0] || engine.second == message.param[1])
                                return "AT+MDINFO=0," + engine.first + "," + engine.second + ",private model has existed";
                        }
                        engine_cloud.emplace_back(message.param[0], message.param[1]);
                        return "AT+MDINFO=0," + engine_cloud.back().first + "," + engine_cloud.back().second;
                    }
                    else if(message.param.size() == 1)
                    {
                        // name的情况和id的情况，都是负责查找
                        std::string temp = message.param.front();
                        size_t pos = temp.find_first_of("_");
                        if(pos == std::string::npos)
                            return "AT+ENVAL";
                        std::string judge = temp.substr(0, pos);
                        temp.erase(0, pos + 1);
                        if(judge == "id")
                        {
                            for(const auto &engine : engine_private)
                                if(temp == engine.second)
                                    return "AT+MDINFO=0," + engine.first + "," + engine.second;
                        }
                        else if(judge == "mid")
                        {
                            for(const auto &engine : engine_cloud)
                                if(temp == engine.second)
                                    return "AT+MDINFO=0," + engine.first + "," + engine.second;
                        }
                        else if(judge == "name")
                        {
                            for(const auto &engine : engine_private)
                                if(temp == engine.first)
                                    return "AT+MDINFO=0," + engine.first + "," + engine.second;
                            for(const auto &engine : engine_cloud)
                                if(temp == engine.first)
                                    return "AT+MDINFO=0," + engine.first + "," + engine.second;
                        }
                        else
                            return "AT+ENVAL";
                        return "AT+MDINFO=-1,the model is not existed";
                    }
                    else 
                        return "AT+ENVAL";
                }
                case Type::SEARCH:
                {
                    // 查找所有的可用的engine模型
                    if(user.empty())
                        return "AT+MDINFO=-4,the service is not registered";
                    std::string result = "AT+MDINFO=";
                    for(const auto &engine : engine_private)
                        result += "(0," + engine.first + "," + engine.second + "),";
                    for(const auto &engine : engine_cloud)
                        result += "(0," + engine.first + "," + engine.second + "),";
                    return std::move(result);
                }
                case Type::LONG:
                {
                    // 设定私有模型
                    if(message.param.size() != 1)
                        return "AT+ENVAL";
                    std::string name = message.param.front();
                    for(const auto &engine : engine_private)
                        if(engine.first == name)
                            return "AT+MDINFO=0," + engine.first + "," + engine.second + ",private model has existed";
                    for(const auto &engine : engine_cloud)
                        if(engine.first == name)
                            return "AT+MDINFO=0," + engine.first + "," + engine.second + ",remote model has existed";
                    std::ofstream outfile;
                    std::string id = create_id();
                    outfile.open("../" + user + "/" + id, std::ios::out | std::ios::binary);
                    if(!outfile.is_open())
                        return "AT+MDINFO=-3," + name + "," + id + ",unable ot open file";
                    outfile.write(message.data.data(), message.data.size());
                    outfile.close();
                    engine_private.emplace_back(name, id);
                    return "AT+MDINFO=0," + engine_private.back().first + "," + engine_private.back().second;
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string MDRM(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::CMD:
                {
                    if(message.param.size() != 1)
                        return "AT+ENVAL";
                    if(user.empty())
                        return "AT+MDMOD=-4,the service is not registered";
                    std::string temp = message.param.front();
                    size_t pos = temp.find_first_of("_");
                    if(pos == std::string::npos)
                        return "AT+ENVAL";
                    std::string judge = temp.substr(0, pos);
                    temp.erase(0, pos + 1);

                    std::string id;

                    if(judge == "mid")
                    {
                        for(auto engine = engine_cloud.begin(); engine != engine_cloud.end(); engine++)
                            if(temp == engine->second)
                            {
                                engine_cloud.erase(engine);
                                return "AT+MDMOD=0";
                            }
                        return "AT+MDMOD=-1,model is not existed";
                    }
                    else if(judge == "id")
                    {
                        for(auto engine = engine_private.begin(); engine != engine_private.end(); engine++)
                            if(temp == engine->second)
                            {
                                id = engine->second;
                                engine_private.erase(engine);
                                break;
                            }
                        if(id.empty())
                            return "AT+MDMOD=-1,model is not existed";
                    }
                    else if(judge == "name")
                    {
                        for(auto engine = engine_private.begin(); engine != engine_private.end(); engine++)
                            if(temp == engine->first)
                            {
                                id = engine->first;
                                engine_private.erase(engine);
                                break;
                            }
                        if(id.empty())
                        {
                            for(auto engine = engine_cloud.begin(); engine != engine_cloud.end(); engine++)
                                if(temp == engine->first)
                                {
                                    engine_cloud.erase(engine);
                                    return "AT+MDMOD=0";
                                }
                            return "AT+MDMOD=-1,model is not existed";
                        } 
                    }
                    else
                        return "AT+ENVAL";
                    if(std::filesystem::remove("../" + user + "/" + id) < 0)
                        return "AT+MDMOD=-5,the index of model is removed";
                    return "AT+MDMOD=0";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;
        using DataRequest = ::jxtd::proto::datatransfer::Request;
        using DataResponse = ::jxtd::proto::datatransfer::Response;
        using DataTask = WFNetworkTask<DataRequest, DataResponse>;
        using DataTaskFactory = WFNetworkTaskFactory<DataRequest, DataResponse>;

        struct DataRequestContext
        {
            std::string src;
            std::string dest;
            std::string token;
            uint32_t direction;
            std::vector<Adtsrc> adtsrcs;
        };

        struct DataSideContext
        {
            std::string addr;
            int port;
            bool ssl;
        };

        using DataServerContext = struct DataSideContext;
        using DataClientContext = struct DataSideContext;
        using DataClientSetupContext = struct DataSideContext;

        class DataClient
        {
        public:
            DataClient();
            ~DataClient();

            void setup(const DataClientSetupContext &ctx);                               // set up client
            SubTask* create(const DataServerContext &ctx, const DataRequestContext &req_ctx, bool &result); // create ready-for-start task

        private:
            DataClientContext *client_ctx;
        };

        DataClient::DataClient()
        {
            this->client_ctx = new DataClientContext;
        }

        DataClient::~DataClient()
        {
            delete this->client_ctx;
        }

        void DataClient::setup(const DataClientSetupContext &ctx)
        {
            this->client_ctx->addr = ctx.addr;
            this->client_ctx->ssl = ctx.ssl;
            this->client_ctx->port = ctx.port;
        }

        SubTask* DataClient::create(const DataServerContext &ctx, const DataRequestContext &req_ctx, bool &result)
        {
            auto task = DataTaskFactory::create_client_task(
                this->client_ctx->ssl ? TT_TCP_SSL : TT_TCP,
                ctx.addr,
                ctx.port,
                1,
                [&result](DataTask *task)
                {
                    if(task->get_state() == WFT_STATE_SUCCESS)
                        result = true;
                });
            auto req = task->get_req();
            req->set_token(req_ctx.token);
            req->set_dest(req_ctx.dest);
            req->set_src(req_ctx.src);
            req->set_direction(req_ctx.direction);
            for(const auto &adtsrc : req_ctx.adtsrcs)
                req->add_adtsrc(adtsrc);
            return task;
        }

        std::string MDPUSH(ATMessage &message)
        {
            if(message.param.size() != 1)
                return "AT+ENVAL";
            if(user.empty())
                return "AT+MDPUSH=-4,the service is not registered";
            DataClient client;
            bool result = false;
            DataRequestContext context;
            std::string router = flag_.datatransfer_router;
            context.src = router;
            size_t pos = router.find_first_of(":");
            if(pos == std::string::npos)
                return "AT+MDPUSH=-5,the source url is incorrect";
            std::string src_url = router.substr(0, pos);
            router.erase(0, pos + 1);
            client.setup({src_url, std::stoi(router), flag_.ssl});
            std::string storage = flag_.datatransfer_storage;
            context.dest = storage;
            pos = storage.find_first_of(":");
            if(pos == std::string::npos)
                return "AT+MDPUSH=-5,the dest url is incorrect";
            std::string dest_url = storage.substr(0, pos);
            storage.erase(0, pos + 1);

            auto push = [&client, &result, dest_url, storage, &context](const std::string &data, const std::string name, const std::string mid)
            {
                // context.token = ""; set token
                context.direction = 0x1;
                Adtsrc adtsrc;
                adtsrc.engineattr.first = name;
                adtsrc.engineattr.second = mid;
                adtsrc.binary = data;
                adtsrc.type = 0x0;
                context.adtsrcs.push_back(adtsrc);
                auto task = client.create({dest_url, std::stoi(storage), flag_.ssl}, context, result);
                WFFacilities::WaitGroup group(1);
                Workflow::start_series_work(task, [&group](const SeriesWork*){group.done();});
                group.wait();
            };

            switch(message.type)
            {
                case Type::CMD:
                {
                    // 没有检查    
                    std::string temp = message.param.front();
                    size_t position = temp.find_first_of("_");
                    if(position  == std::string::npos)
                        return "AT+ENVAL";
                    std::string judge = temp.substr(0, position);
                    temp.erase(0, position + 1);

                    std::string name;
                    std::string mid;
                    std::vector<std::pair<std::string, std::string>>::iterator iter;

                    if(judge == "id")
                    {
                        for(iter = engine_private.begin(); iter != engine_private.end(); iter++)
                            if(temp == iter->second)
                            {
                                mid = iter->second;
                                name = iter->first;
                                break;
                            }
                    }
                    else if(judge == "name")
                    {
                        for(iter = engine_private.begin(); iter != engine_private.end(); iter++)
                            if(temp == iter->first)
                            {
                                mid = iter->second;
                                name = iter->first;
                                break;
                            }
                    }
                    else
                        return "AT+ENVAL";

                    if(name.empty() || mid.empty())
                        return "AT+MDPUSH=-1," + temp + ",the model is not existed";

                    std::ifstream infile;
                    infile.open("../" + user + "/" + mid, std::ios::in | std::ios::binary);
                    if(!infile.is_open())
                        return "AT+MDPUSH=-1," + name + "," + mid + ",the model is not existed";
                    infile.seekg(0, std::ios::end);
                    auto fileSize = infile.tellg();
                    infile.seekg(0, std::ios::beg);
                    std::string data(fileSize, '\0');
                    infile.read(&data[0], fileSize);
                    infile.close();

                    push(data, name, mid);
                    if(!result)
                        return "AT+MDPUSH=-5,unable to push model";
                    
                    engine_private.erase(iter);
                    engine_cloud.emplace_back(name, mid);
                    std::filesystem::remove("../" + user + "/" + mid);

                    return "AT+MDPUSH=0";
                }
                case Type::LONG:
                {
                    std::string name = message.param.front();
                    for(const auto &engine : engine_cloud)
                        if(engine.first == name)
                            return "AT+MDPUSH=-2," + name + "," + engine.second + ",duplicate engine indexes";
                    for(const auto &engine : engine_private)
                        if(engine.first == name)
                            return "AT+MDPUSH=-2," + name + "," + engine.second + ",duplicate engine indexes";
                    std::string mid = create_id();
                    push(message.data, name, mid);
                    if(!result)
                        return "AT+MDPUSH=-5,unable to push model";
                    engine_cloud.emplace_back(name, mid);
                    return "AT+MDPUSH=0,";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        using Engine = ::jxtd::proto::taskdep::Request::Engine;
        using Task = ::jxtd::management::inferprovider::Task;
        using Endpoint = ::jxtd::proto::taskdep::Endpoint;
        using Datasource = ::jxtd::proto::taskdep::Request::Datasource;
        using Attr = ::jxtd::proto::taskdep::Request::Attr;
        using Trigger = ::jxtd::proto::taskdep::Request::Attr_tigger;
        using Directorypolicy = ::jxtd::proto::taskdep::Request::Datasource_dest_directorypolicy;
        using Emptypolicy = ::jxtd::proto::taskdep::Request::Datasource_dest_emptypolicy;

        std::string CPSTART(ATMessage &message)
        {
            if(message.param.size() < 2 || message.param.size() > 3 || (message.type != Type::CMD && message.type != Type::LONG))
                return "AT+ENVAL";
            if(message.param.size() == 3 && message.param[2] != "nonblock")
                return "AT+ENVAL";
            if(user.empty())
                return "AT+CPRES=-4,the service is not registered";
            uint64_t taskid_;
            if((taskid_ = task.get_taskid()) != 0)
                return "AT+CPRES=-9," + std::to_string(taskid_) + "the task is running";
            std::string temp = message.param.front();
            size_t pos = temp.find_first_of("_");
            if(pos == std::string::npos)
                return "AT+ENVAL";
            std::string judge = temp.substr(0, pos);
            temp.erase(0, pos + 1);
            Engine engine;
            bool result = false;
            auto read_ = [&result, &engine](std::string mid)
            {
                std::ifstream infile;
                infile.open("../" + user + "/" + mid, std::ios::in | std::ios::binary);
                if(!infile.is_open())
                    return;
                infile.seekg(0, std::ios::end);
                auto fileSize = infile.tellg();
                infile.seekg(0, std::ios::beg);
                engine.binary = std::string(fileSize, '\0');
                infile.read(&engine.binary[0], fileSize);
                infile.close();
                result = true;
            };

            if(judge == "mid")
            {
                for(const auto &engine_ : engine_cloud)
                    if(engine_.second == temp)
                    {
                        engine.id = engine_.second;
                        engine.name = engine_.first;
                        result = true;
                        break;
                    }
            }
            else if(judge == "id")
            {
                for(const auto &engine_ : engine_private)
                    if(engine_.second == temp)
                    {
                        read_(engine_.second);
                        break;
                    }
            }
            else if(judge == "name")
            {
                for(const auto &engine_ : engine_cloud)
                    if(engine_.first == temp)
                    {
                        engine.id = engine_.second;
                        engine.name = engine_.first;
                        result = true;
                        break;
                    }
                if(!result)
                {
                    for(const auto &engine_ : engine_private)
                        if(engine_.first == temp)
                        {
                            read_(engine_.second);
                            break;
                        }
                }
            }
            else
                return "AT+ENVAL";
            if(!result)
                return "AT+CPRES=-1,the model is not existed";
            uint64_t taskid = management.create_neogiate_task(1, engine);
            if(taskid == 0)
                return "AT+CPRES=-7,cannot get taskid";
            Attr attr;
            attr.trigger = static_cast<uint32_t>(Trigger::create);
            attr.virtstorage = false;
            Datasource datasource;
            datasource.dest.output = "http://localhost:2222";
            datasource.dest.directorypolicy = static_cast<uint32_t>(Directorypolicy::append);
            datasource.dest.emptypolicy = static_cast<uint32_t>(Emptypolicy::create);
            switch(message.type)
            {
                case Type::CMD:
                {
                    datasource.src.input = message.param[1];
                    datasource.src.type = "endpoint";
                    break;
                }
                case Type::LONG:
                {
                    datasource.src.input = std::move(message.data);
                    datasource.src.type = "data";
                    break;
                }
                default:
                {
                    management.create_terminate_task(taskid);
                    return "AT+ENVAL";
                }
            }
            task.set(taskid);
            int result_ = management.create_deploy_task(taskid, datasource, attr, 5); // busy重试次数为5，目前
            if(result_ == -1)
            {
                management.create_terminate_task(taskid);
                return "AT+CPRES=-7,cannot deploy task";
            }
            if(message.param.size() == 2)
            {
                task.wait(); // 如果配置错误就一直阻塞下去了
                if(task.get_status())
                {
                    std::string data = std::move(task.get_data());
                    task.remove();
                    return "AT+CPRES=0,data\r\n" + data + "<<<\r\n";
                }
                task.remove();
                return "AT+CPRES=-8,unable to get result";
            }
            return "AT+CPRES=0," + std::to_string(taskid);
        }

        std::string CPMOD(ATMessage &message)
        {
            if(message.param.size() != 0)
                return "AT+ENVAL";
            switch(message.type)
            {
                case Type::CMD:
                {
                    uint64_t taskid;
                    if((taskid = task.get_taskid()) != 0)
                    {
                        management.create_terminate_task(taskid);
                        task.remove();
                    }
                    return "AT+CPSTA=cancel=true";
                }
                case Type::TEST:
                {
                    if(task.get_taskid() != 0)
                        return "AT+CPSTA=cancel=false";
                    return "AT+CPSTA=cancel=true";
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string CPRDY(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::TEST:
                {
                    if(message.param.size() != 0)
                        return "AT+ENVAL";
                    if(task.get_taskid() != 0)
                        return "AT+BUSY";
                    return "AT+READY"; // 好像和RTBUSY区别不大，这个怎么改
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string CPVAL(ATMessage &message)
        {
            uint64_t sec = 0;
            uint64_t microsec = 0;
            switch(message.type)
            {
                case Type::CMD:
                {
                    if(message.param.size() == 2)
                    {
                        sec = std::stoi(message.param[0]);
                        microsec = std::stoi(message.param[1]);
                    }
                    else if(message.param.size() == 1)
                        sec = std::stoi(message.param[0]);
                    else if(message.param.size() == 0)
                        ;
                    else
                        return "AT+ENVAL";
                    break;
                }
                case Type::TEST:
                {
                    if(message.param.size() != 0)
                        return "AT+ENVAL";
                    break;
                }
                default:
                    return "AT+ENVAL";
            }
            std::string output;
            auto check = [&output](WFTimerTask *task_)
            {
                if(task.get_taskid() == 0)
                {
                    output = "AT+CPRES=-7,the task is canceled";
                    return;
                }
                if(!task.get_status())
                {
                    task.remove();
                    output = "AT+CPRES=-8,cannot get result";
                    return;
                }
                std::string data = task.get_data();
                if(data.empty())
                {
                    output = "AT+CPRES=-9,the task is running";
                    return;
                }
                task.remove();
                output = "AT+CPRES=0,data\r\n" + data + "<<<\r\n";
            };
            auto task = WFTaskFactory::create_timer_task(sec * 1000000 + microsec, check);
            WFFacilities::WaitGroup group(1);
            auto series = Workflow::create_series_work(task, [&group](const SeriesWork* series){
                group.done();
            });
            series->start();
            group.wait();
            return output;
        }

        std::string TRANS(ATMessage &message)
        {
            switch(message.type)
            {
                case Type::LONG:
                {
                    if(message.param.size() != 1)
                        return "AT+ENVAL";
                    size_t len = std::stoi(message.param.at(0));

                    int pipefd[2];
                    pid_t pid;
                    if(pipe(pipefd) == -1)
                        return "Failed to create pipe";
                    pid = fork();

                    if(pid == -1)
                        return "Failed to fork";
                    else if (pid == 0)
                    {
                        close(pipefd[0]);
                        dup2(pipefd[1], STDOUT_FILENO);
                        std::vector<std::string> commands;

                        size_t pos;
                        while((pos = message.data.find_first_of(" ")) != std::string::npos)
                        {
                            commands.push_back(message.data.substr(0, pos));
                            message.data.erase(0, pos + 1);
                        }
                        const char *argv[commands.size() + 1];
                        for (size_t i = 0; i < commands.size(); ++i)
                            argv[i] = commands[i].c_str();
                        argv[commands.size()] = nullptr;
                        execvp(argv[0], const_cast<char**>(argv));
                        close(pipefd[1]);
                    }
                    else
                    {
                        close(pipefd[1]);
                        char buffer[4096];
                        ssize_t bytesRead;
                        std::string output;
                        while((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
                            output += std::string(buffer, bytesRead);

                        int status;
                        waitpid(pid, &status, 0);

                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                            output += "\r\n<<<\r\n";
                        else
                            output += "\r\n<<<Command failed to execute\r\n";
                        close(pipefd[0]);
                        return output;
                    }
                }
                default:
                    return "AT+ENVAL";
            }
        }

        std::string STRANS(ATMessage &message)
        {
            if(message.type != Type::LONG || message.param.size() != 1)
                return "AT+ENVAL";
            size_t pos = message.data.find("");
            std::string command;
            if(pos >= 0)
                command = message.data.substr(0, pos);
            else
                command = message.data;
            if(command != "ip" && command != "ifconfig")
                return "Wrong sudo command";
            int length = std::stoi(message.param.front());
            length += 5;
            message.param.at(0) = std::to_string(length);
            message.data = "sudo " + message.data;
            return TRANS(message);
        }
    }
}