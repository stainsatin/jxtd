#include <gtest/gtest.h>
#include <management/datasource/DataSourceServer.h>
#include <management/datasource/DataSourceList.h>
#include "Registry.h"
#include <sys/socket.h>
#include <cstdint>
TEST(testDataSource, Registry){
    ::jxtd::nvr::datasource::Registry client_registry;
    client_registry.load_config("svr1", "localhost", 0, 6666);
    client_registry.load_config("svr2", "localhost", 0, 7777);
    uint32_t client_proto = ::jxtd::nvr::datasource::Registry::Http;
    uint32_t port1 = 3456;
    uint32_t port2 = 2345;
    ::jxtd::management::datasource::DataSourceServer server1({
        AF_INET,
        "localhost",
        6666,
        false,
        "",
        "",
        ""
    });
    EXPECT_EQ(server1.start(), 0);
    ::jxtd::management::datasource::DataSourceServer server2({
        AF_INET,
        "localhost",
        7777,
        false,
        "",
        "",
        ""
    });
    EXPECT_EQ(server2.start(), 0);
    client_registry.register_datasource(
        "dt1", "localhost",
        client_proto,
        port1, "svr1"
    );
    client_registry.register_datasource(
        "dt1", "localhost",
        client_proto,
        port2, "svr2"
    );

    auto list1 = server1.get_datasource();
    auto list2 = server2.get_datasource();

    EXPECT_TRUE(list1 -> has_datasource("dt1"));
    EXPECT_TRUE(list2 -> has_datasource("dt1"));
    EXPECT_TRUE(
        std::tie(client_proto, "localhost", port1)
        == list1 -> get_datasource("dt1")
    );
    EXPECT_TRUE(
        std::tie(client_proto, "localhost", port2)
        == list2 -> get_datasource("dt1")
    );

    server1.stop();
    server2.stop();

    EXPECT_NE(client_registry.register_datasource("dt2", "localhost", client_proto, port1, "svr1"), 0);
}
