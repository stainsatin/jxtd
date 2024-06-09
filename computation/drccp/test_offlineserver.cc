/**
 * test for multi-server cluster, online gradually, offline occurs, no delay
 */
#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include "server.h"
#include <string>

using DrccpServer = ::jxtd::computation::drccp::DrccpServer;
static DrccpServer* start_server(int port, int boot_port);
static std::string submit(DrccpServer* server, const std::string& payload, const std::string addr, int port);
//just for a single node cluster, no offline, no relay
TEST(testDrccp, testmulti){
    auto server1 = start_server(7777, 7777);
    auto server2 = start_server(8888, 7777);
    auto server3 = start_server(9999, 7777);
    std::string payload = "test";
    EXPECT_EQ(
        payload,
        submit(server3, payload, "localhost", 4444)
    );
    server3 -> stop();
    EXPECT_EQ(
        "",
        submit(server3, payload, "localhost", 5555)
    );
    delete server3;
    delete server2;
    EXPECT_EQ(
        payload,
        submit(server1, payload, "localhost", 3333)
    );
    delete server1;
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
static std::string submit(DrccpServer* server, const std::string& payload, const std::string addr, int port){
    WFFacilities::WaitGroup group(1);
    std::string recv;
    server -> register_client_receive_result([&group, &recv](const std::string& result){
        recv = result;
        group.done();
    });
    server -> submit(payload, {addr, port});
    group.wait();
    return recv;
}
