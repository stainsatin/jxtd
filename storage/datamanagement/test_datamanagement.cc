#include <gtest/gtest.h>
#include "datamanagement.h"
#include <string>

using Datamanager = ::jxtd::storage::datamanagement::Datamanager;
TEST(testDatamanagement, testdatamanager)
{
    Datamanager manager;
    EXPECT_EQ(manager.backend_choose("remote"), -1);
    EXPECT_EQ(manager.backend_choose("local"), 0);
    EXPECT_EQ(manager.init("./test", false), -1);
    EXPECT_EQ(manager.init("./test", true), 0);
    EXPECT_EQ(manager.engine_push("111", "222", "hollow"), 0);
    EXPECT_EQ(manager.engine_pull("111", "22"), "");
    EXPECT_EQ(manager.engine_pull("111", "222"), "hollow");
    EXPECT_EQ(manager.binary_push("a/b/c", "lll"), 0);
    EXPECT_EQ(manager.binary_pull("a/b/c"), "lll");
    manager.deinit();
}
