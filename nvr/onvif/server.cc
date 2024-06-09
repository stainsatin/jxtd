#include "server.h"
#include "adapter.h"
#include <cstdint>
#include <string>
#include <functional>
#include <proto/onvif/message.h>
#include <workflow/WFServer.h>
#include <workflow/WFTaskFactory.h>
namespace jxtd{
    namespace nvr{
        namespace onvif{
            using OnvifForwardRequest = class ::jxtd::proto::onvif::Message;
            using OnvifForwardResponse = OnvifForwardRequest;
            using OnvifForwardTask = WFNetworkTask<OnvifForwardRequest, OnvifForwardResponse>;
            using OnvifForwardWFServer = WFServer<OnvifForwardRequest, OnvifForwardResponse>;
            struct OnvifForwardServerContext{
                OnvifForwardWFServer* server;
                int family;
                int port;
                bool ssl;
                std::string ca;
                std::string cert;
                std::string key;
                uint32_t uid;
                OnvifAdapter* adapter;
            };
            static void process(OnvifForwardTask* task, OnvifForwardServerContext* ctx);
            OnvifForwardServer::OnvifForwardServer()
                :server_ctx(new OnvifForwardServerContext){}
            OnvifForwardServer::OnvifForwardServer(
                const OnvifForwardServerSetupContext& setup
            ):OnvifForwardServer(){
                this -> server_ctx -> server = nullptr;
                this -> server_ctx -> family = setup.family;
                this -> server_ctx -> port = setup.port;
                this -> server_ctx -> ssl = setup.ssl;
                this -> server_ctx -> ca = setup.ca;
                this -> server_ctx -> cert = setup.cert;
                this -> server_ctx -> key = setup.key;
                this -> server_ctx -> uid = setup.uid;
                this -> server_ctx -> adapter = setup.adapter;
            }
            OnvifForwardServer::~OnvifForwardServer(){
                this -> stop();
                delete this -> server_ctx;
            }
            int OnvifForwardServer::start(){
                if(this -> server_ctx -> server)
                    return 0;
                this -> server_ctx -> server = new OnvifForwardWFServer(
                    std::bind(process, std::placeholders::_1, this -> server_ctx)
                );
                if(this -> server_ctx -> ssl)
                    return this -> server_ctx -> server -> start(
                        this -> server_ctx -> family,
                        this -> server_ctx -> port,
                        this -> server_ctx -> cert.c_str(),
                        this -> server_ctx -> key.c_str()
                    );
                else
                    return this -> server_ctx -> server -> start(
                        this -> server_ctx -> family,
                        this -> server_ctx -> port
                    );
            }
            int OnvifForwardServer::stop(){
                if(!this -> server_ctx -> server)
                    return 0;
                this -> server_ctx -> server -> stop();
                delete this -> server_ctx -> server;
                this -> server_ctx -> server = nullptr;
                return 0;
            }
            static void process(OnvifForwardTask* task, OnvifForwardServerContext* ctx){
                auto req = task -> get_req();
                auto resp = task -> get_resp();
                resp -> set_transaction(req -> get_transaction());
                resp -> set_dest(req -> get_src());
                resp -> set_src(ctx -> uid);
                for(const auto& action:req -> get_actions()){
                    if(action.scan){
                        for(const auto& device:ctx -> adapter -> get_devices())
                            resp -> add_action({
                                device.xaddr,
                                device.xaddr,
                                true, true, false, false, false, false,
                                0.0f, 0.0f, 0.0f, 0
                            });
                        return;
                    }
                    const auto& [target, result, ack, scan, streaming, snap, moving, absmov, pan, tilt, zoom, timeout] = action;
                    if(moving)
                        ctx -> adapter -> ptz_device(
                            target, absmov, {pan, tilt, zoom}, timeout
                        );
                    resp -> add_action({
                        target, "",
                        true, scan, streaming, snap, moving, absmov,
                        pan, tilt, zoom, timeout
                    });
                }
            }
        }
    }
}
