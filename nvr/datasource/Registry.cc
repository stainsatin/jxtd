#include <tuple>
#include <stdio.h>
#include <string.h>
#include "Registry.h"
#include <workflow/Workflow.h>
#include "workflow/WFTaskFactory.h"
#include "workflow/WFFacilities.h"
#include "proto/datasource/message.h"

namespace jxtd::nvr::datasource{

    using metaentry = ::jxtd::proto::datasource::metaentry;
    using request = ::jxtd::proto::datasource::request;
    using response = ::jxtd::proto::datasource::response;
    using Request = ::jxtd::proto::datasource::Request;
    using Response = ::jxtd::proto::datasource::Response;
    using WF_dtsr_Task = WFNetworkTask<Request, Response>;
    using callback_t = std::function<void (WF_dtsr_Task *)>;

    class MyFactory : public WFTaskFactory{
    public:
        static WF_dtsr_Task *create_task(const std::string& url,
                                         bool ssl,
                                         unsigned short port,
                                         int retry_max,
                                         callback_t callback)
        {
            using NTF = WFNetworkTaskFactory<Request, Response>;
            auto transport = ssl ? TT_TCP_SSL:TT_TCP;
            WF_dtsr_Task *task = NTF::create_client_task(transport, url,
                                                        port,retry_max,
                                                        std::move(callback));
            return task;
        }
    };

    void Registry::load_config(const std::string& config_name,
                               const std::string& url,
                               bool ssl, uint32_t port)
    {
       this->configMap[config_name] = std::make_tuple(url, ssl, port);
    }

    uint32_t Registry::register_datasource(const std::string& dtname,
                                           const std::string& url,
                                           uint32_t proto,
                                           uint32_t port,
                                           const std::string& config_name)
    {
        uint32_t sym = 0;
        auto config_ = this->configMap.find(config_name);

        if(config_ == configMap.end())
            return -1;

        WFFacilities::WaitGroup wait_group(1);
        WF_dtsr_Task *task = MyFactory::create_task(std::get<0>(config_->second),
                                                   std::get<1>(config_->second),
                                                   std::get<2>(config_->second), 0,
                                                   [&sym, &wait_group, &dtname](WF_dtsr_Task *task){
            int state = task->get_state();
            int error = task->get_error();
            Response *resp = task->get_resp();

            if(state != WFT_STATE_SUCCESS){
                sym = 1;
                wait_group.done();
                return;
            }
            if(resp->get_name() != dtname){
                sym = 2;
                wait_group.done();
                return;
            }
            if(resp->get_status() != Response::status_msg::Status_ok){
                sym = 3;
                wait_group.done();
                return;
            }

            sym = 0;
            wait_group.done();
        });

        auto req = task->get_req();
        req->set_name(dtname);
        req->set_url(url);
        req->set_port(port);
        req->set_action(Request::action::Register);
        req->set_proto(proto);

        task->start();
        wait_group.wait();

        return sym;
    }

}
