/**
 * test for multi-server cluster, online gradually, no offline, no delay
 */
#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include "server.h"
#include <string>

using DrccpServer = ::jxtd::computation::drccp::DrccpServer;
static DrccpServer* start_server(int port, int boot_port);
//just for a single node cluster, no offline, no relay
TEST(testDrccp, testmulti){
    auto server1 = start_server(7777, 7777);
    auto server2 = start_server(8888, 7777);
    std::string payload = "test";
    std::string recv;
    WFFacilities::WaitGroup group1(1);
    server2 -> register_client_receive_result([&group1, &recv](const std::string& result){
        recv = result;
        group1.done();
    });
    server2 -> submit(payload, {"localhost", 8888});
    group1.wait();
    EXPECT_EQ(recv, payload);
    recv = "";
    auto server3 = start_server(9999, 7777);
    WFFacilities::WaitGroup group2(1);
    server3 -> register_client_receive_result([&group2, &recv](const std::string& result){
        recv = result;
        group2.done();
    });
    server3 -> submit(payload, {"localhost", 9999});
    group2.wait();
    EXPECT_EQ(recv, payload);
    delete server1;
    delete server2;
    delete server3;
}
static DrccpServer* start_server(int port, int boot_port){
    auto server = new ::jxtd::computation::drccp::DrccpServer();
    server -> setup({
        "localhost", AF_INET, port,
        false,
        "", "", ""
    });
    server -> register_receive_payload([](std::string payload){return payload;});
    server -> start();
    server -> signin("cluster", "localhost", boot_port);
    return server;
}
