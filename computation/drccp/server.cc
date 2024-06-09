#include "server.h"
#include "message.h"
#include "drccp13.pb.h"
#include <workflow/WFServer.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/Workflow.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
#include <tuple>
#include <string>
#include <functional>

namespace jxtd{
    namespace computation{
        namespace drccp{
            using DrccpRequest = class Message;
            using DrccpResponse = class Message;
            using DrccpTask = WFNetworkTask<DrccpRequest, DrccpResponse>;
            using DrccpWFServer = WFServer<DrccpRequest, DrccpResponse>;
            using DrccpTaskFactory = WFNetworkTaskFactory<DrccpRequest, DrccpResponse>;
            using DrccpEndpoint = std::tuple<std::string, int>;
            using DrccpTaskContext = struct DrccpTaskContext;
            struct DrccpTaskContext{
                DrccpTask* task;
                bool forward_success;
            };
            struct DrccpServerContext{
                DrccpWFServer* server = nullptr;
                std::string addr;
                int family;
                int port;
                std::string cluster_tag;
                std::map<DrccpEndpoint, float> shares;
                bool ssl;
                DrccpServer::on_server_receive_payload_t payload_process = nullptr;
                DrccpServer::on_server_receive_result_t result_process = nullptr;
                std::string cert;
                std::string key;
                std::string ca;
                int response_timeout = -1;
                int request_size_limit = 200*1024*1024;
            };
            struct DrccpClientContext{
                DrccpServer::on_client_receive_result_t result_process = nullptr;
                std::string addr;
                int port;
                bool ssl;
            };
            static uint32_t hashcode(const DrccpEndpoint& src);
            static void setup_header_resp(DrccpTask* task, DrccpServerContext* server_ctx);
            static void setup_share_resp(DrccpTask* task, DrccpServerContext* server_ctx);
            static void setup_dest_resp(DrccpTask* task, const DrccpEndpoint& dest);
            static void setup_raw_resp(DrccpTask* task, const std::string& raw);
            static void setup_header_req(DrccpTask* task, DrccpServerContext* server_ctx, DrccpClientContext* client_ctx);
            static void setup_dest_req(DrccpTask* task, const DrccpEndpoint& dest);
            static void setup_raw_req(DrccpTask* task, const std::string& raw);
            static void process(DrccpTask* task, DrccpServerContext* server_ctx, DrccpClientContext* client_ctx);
            static void update_shares(DrccpServerContext* server_ctx, const DrccpEndpoint& peer, const std::vector<std::tuple<DrccpEndpoint,float>>& shares);
            static DrccpEndpoint select_best_target(DrccpServerContext* ctx, const DrccpEndpoint& source);
            static DrccpTask* forward_task_repeater(DrccpServerContext* ctx, WFRepeaterTask* entrypoint);
            DrccpServer::DrccpServer(){
                this -> server_ctx = new DrccpServerContext;
                this -> client_ctx = new DrccpClientContext;
            }
            DrccpServer::~DrccpServer(){
                this -> stop();
                delete this -> server_ctx;
                delete this -> client_ctx;
            }
            int DrccpServer::setup(const DrccpSetupContext& ctx){
                this -> server_ctx -> addr = ctx.listen_addr;
                this -> server_ctx -> family = ctx.family;
                this -> server_ctx -> port = ctx.port;
                this -> server_ctx -> ssl = ctx.ssl;
                this -> client_ctx -> ssl = ctx.ssl;
                this -> server_ctx -> ca = ctx.ca;
                this -> server_ctx -> cert = ctx.cert;
                this -> server_ctx -> key = ctx.key;
                return 0;
            }
            int DrccpServer::start(){
                auto configs = HTTP_SERVER_PARAMS_DEFAULT;
                configs.peer_response_timeout = this -> server_ctx -> response_timeout;
                configs.request_size_limit = this -> server_ctx -> request_size_limit;
                this -> server_ctx -> server = new DrccpWFServer(
                    &configs,
                    std::bind(process, std::placeholders::_1, this -> server_ctx, this -> client_ctx)
                );
                this -> server_ctx -> shares.insert({
                    {this -> server_ctx -> addr, this -> server_ctx -> port},
                    1.0f
                });
                if(this -> server_ctx -> ssl)
                    return this -> server_ctx -> server -> start(
                        this -> server_ctx -> family,
                        this -> server_ctx -> addr.c_str(),
                        this -> server_ctx -> port,
                        this -> server_ctx -> cert.c_str(),
                        this -> server_ctx -> key.c_str()
                    );
                else
                    return this -> server_ctx -> server -> start(
                        this -> server_ctx -> family,
                        this -> server_ctx -> addr.c_str(),
                        this -> server_ctx -> port
                    );
            }
            int DrccpServer::stop(){
                if(this -> server_ctx -> server){
                    this -> server_ctx -> server -> stop();
                    delete this -> server_ctx -> server;
                    this -> server_ctx -> server = nullptr;
                }
                return 0;
            }
            int DrccpServer::signin(const std::string& cluster_tag,
                                    const std::string& boot_addr,
                                    int boot_port){
                WFFacilities::WaitGroup group(1);
                bool result = false;
                this -> server_ctx -> cluster_tag = cluster_tag;
                auto meet_task = DrccpTaskFactory::create_client_task(
                    this -> server_ctx -> ssl?TT_TCP_SSL:TT_TCP,
                    boot_addr, boot_port,
                    1,
                    [&group, &result, &boot_addr, &boot_port, this](DrccpTask* task){
                        if(task -> get_state() != WFT_STATE_SUCCESS
                            || task -> get_resp() -> gettype() == Report::VERSION){
                            group.done();
                            return;
                        }
                        auto resp = task -> get_resp();
                        for(const auto& [neigh, share]:resp -> getshares())
                            server_ctx -> shares.insert({neigh, share});
                        result = true;
                        group.done();
                    }
                );
                this -> client_ctx -> addr = this -> server_ctx -> addr;
                this -> client_ctx -> port = this -> server_ctx -> port;
                setup_header_req(meet_task, this -> server_ctx, this -> client_ctx);
                setup_dest_req(meet_task, {boot_addr, boot_port});
                meet_task -> get_req() -> settype(Report::MEET);
                meet_task -> start();
                group.wait();
                if(result)
                    return 0;
                this -> server_ctx -> cluster_tag = "";
                return -1;
            }
            int DrccpServer::logout(){
                this -> server_ctx -> cluster_tag = "";
                return 0;
            }
            int DrccpServer::submit(const std::string& payload, const DrccpProxyOriginContext& ctx){
                this -> client_ctx -> addr = ctx.origin_addr;
                this -> client_ctx -> port = ctx.origin_port;
                auto local_task = DrccpTaskFactory::create_client_task(
                    this -> client_ctx -> ssl?TT_TCP_SSL:TT_TCP,
                    this -> server_ctx -> addr,
                    this -> server_ctx -> port,
                    1,
                    [this](DrccpTask* task){
                        if(task -> get_state() != WFT_STATE_SUCCESS
                            || task -> get_resp() -> gettype() == Report::VERSION){
                                if(client_ctx -> result_process)
                                    client_ctx -> result_process("");
                        }
                        auto resp = task -> get_resp();
                        if(client_ctx -> result_process)
                            client_ctx -> result_process(task -> get_resp() -> getraw());
                    }
                );
                setup_header_req(local_task, this -> server_ctx, this -> client_ctx);
                setup_dest_req(local_task, {this -> server_ctx -> addr, this -> server_ctx -> port});
                setup_raw_req(local_task, payload);
                local_task -> get_req() -> settype(Report::LOCAL);
                local_task -> start();
                return 0;
            }
            void DrccpServer::remove_receive_payload(){
                this -> server_ctx -> payload_process = nullptr;
            }
            void DrccpServer::register_receive_payload(on_server_receive_payload_t callback){
                this -> server_ctx -> payload_process = callback;
            }
            void DrccpServer::remove_receive_result(){
                this -> server_ctx -> result_process = nullptr;
            }
            void DrccpServer::register_receive_result(on_server_receive_result_t callback){
                this -> server_ctx -> result_process = callback;
            }
            void DrccpServer::register_client_receive_result(on_client_receive_result_t callback){
                this -> client_ctx -> result_process = callback;
            }
            void DrccpServer::remove_client_receive_result(){
                this -> client_ctx -> result_process = nullptr;
            }
            static uint32_t hashcode(const DrccpEndpoint& src){
                const auto& [addr, port] = src;
                uint32_t code = 0x0;
                for(int i = 0;i < addr.size();i++){
                    int dat = addr[i];
                    code = ((code * 31) + dat) ^ (port << (i%32));
                }
                return code;
            }
            static void setup_header_resp(DrccpTask* task, DrccpServerContext* server_ctx){
                auto resp = task -> get_resp();
                resp -> setversion(3);
                resp -> settag(server_ctx -> cluster_tag, "unused");
                resp -> setsrc(server_ctx -> addr, server_ctx -> port);
            }
            static void setup_share_resp(DrccpTask* task, DrccpServerContext* server_ctx){
                auto resp = task -> get_resp();
                for(const auto& [neigh, share]:server_ctx -> shares)
                    resp -> addshare({neigh, share});
            }
            static void setup_dest_resp(DrccpTask* task, const DrccpEndpoint& dest){
                auto resp = task -> get_resp();
                const auto& [addr, port] = dest;
                resp -> setdest(addr, port);
            }
            static void setup_raw_resp(DrccpTask* task, const std::string& raw){
                task -> get_resp() -> setraw(raw);
            }
            static void setup_header_req(DrccpTask* task, DrccpServerContext* server_ctx, DrccpClientContext* client_ctx){
                auto req = task -> get_req();
                req -> setversion(3);
                req -> settag(server_ctx -> cluster_tag, "unused");
                req -> setsrc(client_ctx -> addr, client_ctx -> port);
            }
            static void setup_dest_req(DrccpTask* task, const DrccpEndpoint& dest){
                const auto& [addr, port] = dest;
                task -> get_req() -> setdest(addr, port);
            }
            static void setup_raw_req(DrccpTask* task, const std::string& raw){
                task -> get_req() -> setraw(raw);
            }
            static void process(DrccpTask* task, DrccpServerContext* server_ctx, DrccpClientContext* client_ctx){
                auto req = task -> get_req();
                auto resp = task -> get_resp();
                auto type = static_cast<Report_MessageType>(req -> gettype());
                //pre-response check, cluster_tag and unknown type
                const auto& [cluster_tag, msg_tag] = req -> gettag();
                if(cluster_tag != server_ctx -> cluster_tag){
                    task -> noreply();
                    return;
                }
                if(type == Report::UNKNOWN){
                    task -> noreply();
                    return;
                }
                //now response is required, setup initial attributes
                setup_header_resp(task, server_ctx);
                setup_dest_resp(task, req -> getsrc());
                //pre-type check, version
                auto version = req -> getversion();
                if(version != 3){
                    resp -> settype(Report::VERSION);
                    return;
                }
                //type scheduler
                switch(type){
                    case Report::MEET:
                        //add new share to shares
                        server_ctx -> shares.insert({req -> getsrc(), 1.0f});
                        //fall down to hello
                    case Report::HELLO:
                        //add shares to response, and reply
                        update_shares(server_ctx, req -> getsrc(), req -> getshares());
                        resp -> settype(Report::ALIVE);
                        break;
                    case Report::LOCAL:
                        //forward task, and return result
                        series_of(task) -> push_back(WFTaskFactory::create_repeater_task(
                            std::bind(forward_task_repeater, server_ctx, std::placeholders::_1), nullptr)
                        );
                        break;
                    case Report::FORWARD:
                        //execute computation task, and reply to src
                        resp -> settype(Report::RESULT);
                        resp -> setraw(server_ctx -> payload_process(req -> getraw()));
                        server_ctx -> shares[{server_ctx -> addr, server_ctx -> port}] += 0.5f;
                        break;
                }
                //push back update task
                if(type != Report::HELLO)
                    for(const auto& [neigh, share] : server_ctx -> shares){
                        if(neigh == std::tie(server_ctx -> addr, server_ctx -> port))
                            continue;
                        const auto& [neigh_addr, neigh_port] = neigh;
                        auto update_task = DrccpTaskFactory::create_client_task(
                            client_ctx -> ssl?TT_TCP_SSL:TT_TCP,
                            neigh_addr, neigh_port,
                            1,
                            [&neigh, &neigh_addr, &neigh_port, &server_ctx](DrccpTask* task){
                                if(task -> get_state() != WFT_STATE_SUCCESS){
                                    server_ctx -> shares[neigh] = 0.0f;
                                    return;
                                }
                                update_shares(server_ctx, neigh, task -> get_resp() -> getshares());
                            }
                        );
                        auto update_req = update_task -> get_req();
                        client_ctx -> addr = server_ctx -> addr;
                        client_ctx -> port = server_ctx -> port;
                        setup_header_req(update_task, server_ctx, client_ctx);
                        setup_dest_req(update_task, neigh);
                        update_req -> settype(Report::HELLO);
                        series_of(task) -> push_back(update_task);
                    }
                //install ctx
                auto ctx = new DrccpTaskContext;
                series_of(task) -> set_context(ctx);
                ctx -> task = task;
                ctx -> forward_success = false;
                series_of(task) -> set_callback([](const SeriesWork* series){
                    auto ctx = (DrccpTaskContext*)(series -> get_context());
                    delete ctx;
                });
            }
            static void update_shares(DrccpServerContext* server_ctx, const DrccpEndpoint& peer, const std::vector<std::tuple<DrccpEndpoint,float>>& shares){
                auto iter = server_ctx -> shares.find(peer);
                float self_share, peer_share;
                if(iter != server_ctx -> shares.end())
                    peer_share = iter -> second;
                else
                    peer_share = 1.0f;
                self_share = server_ctx -> shares.find({server_ctx -> addr, server_ctx -> port}) -> second;
                for(const auto& [neigh, neigh_share] : shares){
                    auto iter = server_ctx -> shares.find(neigh);
                    if(iter != server_ctx -> shares.end())
                        server_ctx -> shares[neigh] = (iter -> second * self_share + neigh_share * peer_share) / (1.5 * (self_share + peer_share));
                    else
                        server_ctx -> shares.insert({neigh, neigh_share/1.5});
                }
            }
            static DrccpEndpoint select_best_target(DrccpServerContext* ctx, const DrccpEndpoint& source){
                auto server_ctx = *ctx;
                std::map<uint32_t, DrccpEndpoint> hash_cycle;
                for(const auto& [endpoint, share] : server_ctx.shares){
                    int node_limit = share * 4;
                    auto basehash = hashcode(endpoint);
                    for(int i = 0;i < node_limit;i++){
                        auto interval = (~0x0u)/node_limit;
                        hash_cycle.insert({(basehash + interval * i), endpoint});
                    }
                }
                auto code = hashcode(source);
                auto iter = hash_cycle.find(code);
                if(iter != hash_cycle.end())
                    return iter -> second;
                for(code = code + 1;code != code;code++){
                    auto iter = hash_cycle.find(code);
                    if(iter != hash_cycle.end())
                        return iter -> second;
                }
                return {server_ctx.addr, server_ctx.port};
            }
            static DrccpTask* forward_task_repeater(DrccpServerContext* ctx, WFRepeaterTask* entrypoint){
                auto task_ctx = (DrccpTaskContext*)series_of(entrypoint) -> get_context();
                if(task_ctx -> forward_success)
                    return nullptr;
                auto origin_task = task_ctx -> task;
                auto req = origin_task -> get_req();
                auto [target_addr, target_port] = select_best_target(ctx, req -> getsrc());
                auto forward_task = DrccpTaskFactory::create_client_task(
                    ctx -> ssl ? TT_TCP_SSL:TT_TCP,
                    target_addr, target_port,
                    1,
                    [task_ctx, target_addr, target_port, ctx](DrccpTask* task){
                        DrccpEndpoint target = {target_addr, target_port};
                        if(task -> get_state() != WFT_STATE_SUCCESS){
                            task_ctx -> forward_success = false;
                            ctx -> shares[target] = 0.0f;
                            return;
                        }
                        auto resp = task -> get_resp();
                        if(ctx -> result_process)
                            ctx -> result_process(resp -> getraw());
                        task_ctx -> task -> get_resp() -> settype(Report::RESULT);
                        task_ctx -> task -> get_resp() -> setraw(resp -> getraw());
                        task_ctx -> forward_success = true;
                        ctx -> shares[target] += 0.5f;
                    }
                );
                auto forward_req = forward_task -> get_req();
                forward_req -> setversion(3);
                forward_req -> settag(ctx -> cluster_tag, "unused");
                forward_req -> setsrc(ctx -> addr, ctx -> port);
                forward_req -> setdest(target_addr, target_port);
                forward_req -> setraw(req -> getraw());
                forward_req -> settype(Report::FORWARD);
                return forward_task;
            }
        }
    }
}
