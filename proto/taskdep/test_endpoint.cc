/**
 * test for endpoint
 */
#include <gtest/gtest.h>
#include "proto/taskdep/message.h"
#include <string>

using Endpoint = ::jxtd::proto::taskdep::Endpoint;

TEST(testTaskdep, testendpoint)
{
    Endpoint endpoint;
    EXPECT_EQ(endpoint.set_proto("ssl"), -1);
    EXPECT_EQ(endpoint.set_proto("file"), 0);
    endpoint.set_url("1");
    endpoint.set_port(11);
    endpoint.set_path("/111");
    EXPECT_EQ(endpoint.get_proto(), "file");
    EXPECT_EQ(endpoint.get_url(), "1");
    EXPECT_EQ(endpoint.get_port(), static_cast<uint32_t>(11));
    EXPECT_EQ(endpoint.get_path(), "/111");
    EXPECT_EQ(endpoint.set_authtype(0x0), 0);
    EXPECT_EQ(endpoint.check(), true);
    EXPECT_EQ(endpoint.set_authtype(0x7), -1);
    EXPECT_EQ(endpoint.set_authtype(0x1), 0);
    endpoint.add_authstring_token("1111");
    EXPECT_EQ(endpoint.check(), true);
}
