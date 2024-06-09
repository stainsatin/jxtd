#include <gtest/gtest.h>
#include "storage/taskdep/taskdep.h"
#include "management/taskdep/taskdep.h"
#include "proto/taskdep/message.h"
#include <workflow/WFFacilities.h>

using Endpoint = ::jxtd::proto::taskdep::Endpoint;
using Management_Server = ::jxtd::storage::taskdep::StorageServer;
using Management_Client = ::jxtd::management::taskdep::ManagementClient;
using TaskdepClientContext = ::jxtd::management::taskdep::TaskdepClientContext;
using Engine = ::jxtd::proto::taskdep::Request::Engine;
using Datamanager = ::jxtd::storage::datamanagement::Datamanager;

TEST(testTaskdep, testtaskdep)
{
    Datamanager manager;
    std::mutex task_mutex;
    EXPECT_EQ(manager.init("./test", true), 0);
    TaskdepClientContext context;
    context.ssl = false;
    Endpoint client, server;
    client.set_url("localhost");
    server.set_url("localhost");
    client.set_port(1999);
    server.set_port(2000);
    EXPECT_EQ(client.set_proto("file"), 0);
    EXPECT_EQ(server.set_proto("file"), 0);
    client.set_path("1");
    server.set_path("2");
    EXPECT_EQ(client.set_authtype(0x0), 0);
    EXPECT_EQ(server.set_authtype(0x0), 0);
    Management_Client client_;
    Management_Server server_;
    context.management = client;
    context.storage = server;
    context.computation = client;
    EXPECT_EQ(server_.setup({server, false, AF_INET, "", "", ""}), 0);
    EXPECT_EQ(client_.setup(context), 0);
    EXPECT_EQ(server_.start(manager, task_mutex), 0);
    Engine engine;
    engine.id = "2";
    engine.name = "3";
    engine.deps.push_back(engine);
    engine.extfiles.push_back({client, ""});
    EXPECT_EQ(client_.create_neogiate_task(1, engine), 1);
    EXPECT_TRUE(manager.task_exist(1));
    EXPECT_FALSE(manager.get_task_engine(1).second.empty());
    EXPECT_TRUE(manager.extfiles_exist(1));
    auto taskid = client_.create_neogiate_task(1, engine);
    EXPECT_NE(taskid, 1);
    EXPECT_NE(taskid, 0);
    EXPECT_TRUE(manager.task_exist(taskid));
    EXPECT_TRUE(manager.extfiles_exist(taskid));
    engine.binary = "123123";
    EXPECT_EQ(client_.create_neogiate_task(2, engine), 2);
    EXPECT_TRUE(manager.task_exist(2));
    EXPECT_TRUE(manager.extfiles_exist(2));
    EXPECT_EQ(manager.engine_pull("0000aaaa", "2"), "123123");
    client.set_port(2001);
    client.set_url("localhost2");
    EXPECT_EQ(client_.create_terminate_task(2), 0);
    EXPECT_EQ(client_.create_terminate_task(taskid), 0);
    EXPECT_EQ(client_.create_terminate_task(1), 0);
    EXPECT_FALSE(manager.task_exist(2));
    EXPECT_FALSE(manager.task_exist(1));
    EXPECT_FALSE(manager.task_exist(taskid));
    EXPECT_FALSE(manager.extfiles_exist(2));
    EXPECT_FALSE(manager.extfiles_exist(1));
    EXPECT_FALSE(manager.extfiles_exist(taskid));
    EXPECT_EQ(manager.engine_pull("0000aaaa", "2"), "");
    manager.deinit();
}
