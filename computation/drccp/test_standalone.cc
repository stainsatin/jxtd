/**
 * test for standalone server
 */
#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include "server.h"
#include <string>
//just for a single node cluster, no offline, no relay
TEST(testDrccp, testStandalone){
    ::jxtd::computation::drccp::DrccpServer server;
    server.setup({
        "localhost", AF_INET, 8888,
        false,
        "", "", ""
    });
    ASSERT_EQ(server.start(), 0);
    ASSERT_EQ(server.signin("standalone", "localhost", 8888), 0);
    server.register_receive_payload([](const std::string& raw){return raw;});
    WFFacilities::WaitGroup group(1);
    std::string payload = "test";
    std::string recv;
    server.register_client_receive_result([&group, &recv](const std::string& result){
        recv = result;
        group.done();
    });
    server.submit(payload, {"localhost", 8888});
    group.wait();
    EXPECT_EQ(payload, recv);
}
