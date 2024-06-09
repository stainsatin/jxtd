#include "DatatranServer.h"
#include <mutex>
#include <storage/datamanagement/datamanagement.h>
#include <proto/datatransfer/message.h>
#include <vector>
#include <workflow/WFServer.h>
#include <functional>
#include <string>
namespace jxtd {
    namespace storage{
        namespace datatransfer{
            using DatatransRequest = ::jxtd::proto::datatransfer::Request;
            using DatatransResponse = ::jxtd::proto::datatransfer::Response;
            using DatatransWFServer = WFServer<DatatransRequest, DatatransResponse>;
            using DatatransTask = WFNetworkTask<DatatransRequest, DatatransResponse>;
            using Datamanager = ::jxtd::storage::datamanagement::Datamanager;
            using Dtsrc = ::jxtd::proto::datatransfer::Dtsrc;
            using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;
            static void process(DatatransTask* task, Datamanager& adm, std::mutex& mtx);

            struct DatatranServerContext{
                DatatransWFServer* server = nullptr;
                int family;
                std::string addr;
                int port;
                bool ssl;
                std::string ca_path;
                std::string key_path;
                std::string cert_path;
            };

            enum datatype{
                type_engine = 0x0,
                type_endpoint = 0x1,
                type_binary = 0x2
            };

            DatatranServer::DatatranServer():ctx_(new DatatranServerContext){};

            DatatranServer::~DatatranServer(){
                if(this->ctx_->server)
                    this->stop();
                delete this->ctx_;
            }

            DatatranServer::DatatranServer(const DatatranServerSetupContext& ctx): DatatranServer(){
                setup_context(ctx);
            }

            void DatatranServer::setup_context(const DatatranServerSetupContext& ctx){
                this->ctx_->family = ctx.family;
                this->ctx_->addr = ctx.addr;
                this->ctx_->port = ctx.port;
                this->ctx_->ssl = ctx.ssl;
                this->ctx_->ca_path = ctx.ca_path;
                this->ctx_->key_path = ctx.key_path;
                this->ctx_->cert_path = ctx.cert_path;
            }

            int DatatranServer::start(Datamanager& adm, std::mutex& mtx){
                this->ctx_->server = new DatatransWFServer(
                    std::bind(process, std::placeholders::_1,
                              std::ref(adm), std::ref(mtx))
                );
                if(this->ctx_->ssl)
                    return this->ctx_->server->start(
                        ctx_->family,
                        ctx_->addr.c_str(),
                        ctx_->port,
                        ctx_->cert_path.c_str(),
                        ctx_->key_path.c_str()
                );
                return this->ctx_->server->start(
                    ctx_->family,
                    ctx_->addr.c_str(),
                    ctx_->port
                );
            }

            int DatatranServer::stop(){
                if(!this->ctx_->server)
                    return 0;
                this->ctx_->server->stop();
                delete this->ctx_->server;
                this->ctx_->server = nullptr;
                return 0;
            }
            //以下为process调用的函数
            /*
            struc Adtsrc:taskid type-0x0-engine, 0x1-endpoint/datasource, 0x2-binary
                engineattr---first-mid, second-name
                endpointattr---first-proto, second-url, binary;
            struc Dtsrc:Adtsrc std::vector<std::pair<Adtsrc, std::string>> deps;
            */
            Dtsrc EnginePull(const Adtsrc& master_, Datamanager& adm){
                Dtsrc temp;
                auto taskid_ = master_.taskid;
                
                if(taskid_ == 0){
                    //只设置了Dtsrc的Adtsrc，deps空置
                    auto mid_ = master_.engineattr.first;
                    auto name_ = master_.engineattr.second;
                    temp.master.engineattr = std::make_pair(mid_, name_);
                    temp.master.binary = adm.engine_pull(mid_, name_);
                }
                else{
                    if(adm.task_exist(taskid_) == true){
                        auto t_engine_ = adm.get_task_engine(taskid_);
                        auto mid_name_ = t_engine_.first;
                        auto engine_ = t_engine_.second;
                        //Dtsrc的master
                        auto mid_ = mid_name_.first;
                        auto name_ = mid_name_.second;
                        temp.master.engineattr = std::make_pair(mid_, name_);
                        temp.master.binary = adm.engine_pull(mid_, name_);

                        for(auto eng : engine_){
                            Adtsrc tem;
                            std::pair<Adtsrc, std::string> dep;
                            tem.type = type_engine;
                            tem.engineattr = std::make_pair(eng.id, eng.name);
                            tem.binary = adm.engine_pull(eng.id, eng.name);
                            dep = std::make_pair(tem, "./JXTDENGINE_" + mid_ + "_" + name_ + ".so");
                            temp.deps.emplace_back(dep);
                        }
                    }
                    else{
                        temp.master.binary = "";
                    }
                    //自行处理:设置binary为空，也不管dep
                }
                temp.master.taskid = taskid_;
                temp.master.type = type_engine;
                return temp;
            }

            void EnginePush(const Adtsrc& master_, Datamanager& adm){
                auto mid_ = master_.engineattr.first;
                auto name_ = master_.engineattr.second;
                adm.engine_push(mid_, name_, master_.binary);
            }

            Dtsrc RuntimePull(const Adtsrc& master_, Datamanager& adm){
                Dtsrc temp;
                auto taskid_ = master_.taskid;
                auto proto_ = master_.endpointattr.first;
                auto url_ = master_.endpointattr.second;
                temp.master.taskid = taskid_;
                temp.master.endpointattr = std::make_pair(proto_, url_);
                temp.master.type = type_endpoint;
                temp.master.binary = adm.runtime_pull(taskid_);
                return temp;
            }

            void RuntimePush(const Adtsrc& master_, Datamanager& adm){
                auto proto_ = master_.endpointattr.first;
                auto url_ = master_.endpointattr.second;
                adm.runtime_push(master_.taskid, master_.binary);
            }

            Dtsrc BinaryPull(const Adtsrc& master_, Datamanager& adm){
                Dtsrc temp;
                temp.master.taskid = master_.taskid;
                temp.master.type = type_binary;
                temp.master.binary = adm.binary_pull(adm.get_root_path());
                return temp;
            }

            void BinaryPush(const Adtsrc& master_, Datamanager& adm){
                //有点不对,测试没有这部分
                adm.binary_push(adm.get_root_path(), master_.binary);
            }

            static void process(DatatransTask* task, Datamanager& adm,
                                std::mutex& mtx){
                auto req = task->get_req();
                auto resp = task->get_resp();
                resp->set_src(req->get_dest());
                resp->set_dest(req->get_src());
                resp->set_token("");
                for(auto master_ : req->get_adtsrcs()){
                    //0x0--engine,0x1--endpoint,0x2--binary
                    Dtsrc temp_;
                    std::lock_guard<std::mutex> lock(mtx);
                    switch(master_.type){
                    case type_engine:
                        if(!req->get_direction()){
                            temp_ = EnginePull(master_, adm);
                            resp->add_dtsrc(temp_);
                        }
                        else{
                            EnginePush(master_, adm);
                        }
                        break;
                    case type_endpoint:
                        if(!req->get_direction()){
                            temp_ = RuntimePull(master_, adm);
                            resp->add_dtsrc(temp_);
                        }
                        else{
                            RuntimePush(master_, adm);
                        }
                        break;
                    case type_binary:
                        if(!req->get_direction()){
                            temp_ = BinaryPull(master_, adm);
                            resp->add_dtsrc(temp_);
                        }
                        else{
                            BinaryPush(master_, adm);
                        }
                        break;
                    }
                }
            }
        }
    }
}