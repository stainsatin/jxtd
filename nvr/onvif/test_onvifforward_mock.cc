#include "adapter.h"
#include "server.h"
#include <proto/onvif/message.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

class OnvifAdapterMock: public jxtd::nvr::onvif::OnvifAdapter{
    public:
        MOCK_METHOD((std::vector<jxtd::nvr::onvif::OnvifAdapter::OnvifDevice>), devices_list, (), (override));
        MOCK_METHOD(int32_t, absolute_move,
            ((const std::string&), (const std::tuple<float, float, float>&)),
            (override)
        );
        MOCK_METHOD(int32_t, continous_move,
            ((const std::string&), (const std::tuple<float, float, float>&), uint16_t),
            (override)
        );
};

using TOnvifForwardRequest = class ::jxtd::proto::onvif::Message;
using TOnvifForwardResponse = TOnvifForwardRequest;
using TOnvifForwardTask = WFNetworkTask<TOnvifForwardRequest, TOnvifForwardResponse>;
using TOnvifForwardTaskFactory = WFNetworkTaskFactory<TOnvifForwardRequest, TOnvifForwardResponse>;
static TOnvifForwardTask* create_client_task(const std::string addr, int port);

TEST(onvifforward, dispatcher){
    OnvifAdapterMock mock;
    std::vector<::jxtd::nvr::onvif::OnvifAdapter::OnvifDevice> scan_actions = {
        {"ipc1", 80, "", "", {0.0f, 0.0f, 0.0f}}
    };
    EXPECT_CALL(mock, devices_list())
        .WillOnce(::testing::Return(scan_actions));
    EXPECT_CALL(mock, absolute_move("ipc1", ::testing::_))
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(mock, continous_move("ipc1", ::testing::_, ::testing::_))
        .WillOnce(::testing::Return(0));

    jxtd::nvr::onvif::OnvifForwardServer server({
        0, TT_TCP, 6666, false, "", "", "", &mock
    });

    server.start();

    //scan
    auto task = create_client_task("localhost", 6666);
    WFFacilities::WaitGroup group1(1);
    auto req = task -> get_req();
    req -> set_transaction(0);
    req -> set_src(1);
    req -> set_dest(0);
    req -> set_token("");
    req -> set_ext("");
    req -> add_action({
        "", "", false, true, false, false, false, false, 0.0f, 0.0f, 0.0f, 0
    });
    std::string ipc_addr;
    task -> set_callback([&ipc_addr, &group1](TOnvifForwardTask* task){
        auto resp = task -> get_resp();
        ipc_addr = resp -> get_action(0).target;
        group1.done();
    });
    task -> start();
    group1.wait();


    //moving
    task = create_client_task("localhost", 6666);
    WFFacilities::WaitGroup group2(1);
    req = task -> get_req();
    req -> set_transaction(0);
    req -> set_src(1);
    req -> set_dest(0);
    req -> set_token("");
    req -> set_ext("");
    req -> add_action({
        ipc_addr, "", false, false, false, false, true, false, 0.0f, 0.0f, 0.0f, 0
    });
    task -> set_callback([&group2](TOnvifForwardTask* task){
        group2.done();
    });
    task -> start();
    group2.wait();

    //absmov
    task = create_client_task("localhost", 6666);
    WFFacilities::WaitGroup group3(1);
    req = task -> get_req();
    req -> set_transaction(0);
    req -> set_src(1);
    req -> set_dest(0);
    req -> set_token("");
    req -> set_ext("");
    req -> add_action({
        ipc_addr, "", false, false, false, false, true, true, 0.0f, 0.0f, 0.0f, 0
    });
    task -> set_callback([&group3](TOnvifForwardTask* task){
        group3.done();
    });
    task -> start();
    group3.wait();

    server.stop();
}
static TOnvifForwardTask* create_client_task(const std::string addr, int port){
    auto task = TOnvifForwardTaskFactory::create_client_task(
        TT_TCP, addr, port, 1, nullptr
    );
    return task;
}
