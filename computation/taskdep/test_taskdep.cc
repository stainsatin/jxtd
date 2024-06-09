#include <gtest/gtest.h>
#include "computation/taskdep/taskdep.h"
#include "management/taskdep/taskdep.h"
#include "proto/taskdep/message.h"
#include <workflow/WFFacilities.h>

using Endpoint = ::jxtd::proto::taskdep::Endpoint;
using Cluster_Server = ::jxtd::computation::taskdep::ClusterServer;
using Management_Client = ::jxtd::management::taskdep::ManagementClient;
using Attr = ::jxtd::proto::taskdep::Request::Attr;
using Datasource = ::jxtd::proto::taskdep::Request::Datasource;
using Trigger = ::jxtd::proto::taskdep::Request::Attr_tigger;
using Deploy_status = ::jxtd::proto::taskdep::Response::Deploy_status;
using Deploy_taskstatus = ::jxtd::proto::taskdep::Response::Deploy_taskstatus;
using TaskdepClientContext = ::jxtd::management::taskdep::TaskdepClientContext;
using Taskmanager = ::jxtd::computation::infer::Taskmanager;
using Servermanager = ::jxtd::computation::infer::DrccpServerManager;

TEST(testTaskdep, testtaskdep)
{
    TaskdepClientContext context;
    Endpoint client, server;
    client.set_url("localhost");
    server.set_url("localhost");
    client.set_port(1999);
    server.set_port(2000);
    EXPECT_EQ(client.set_proto("jxtd"), 0);
    EXPECT_EQ(server.set_proto("jxtd"), 0);
    client.set_path("1");
    server.set_path("2");
    EXPECT_EQ(client.set_authtype(0x0), 0);
    EXPECT_EQ(server.set_authtype(0x0), 0);
    Management_Client client_;
    Cluster_Server server_;
    Taskmanager t_manager;
    Servermanager s_manager;
    context.ssl = false;
    context.computation = server;
    context.management = client;
    context.storage = client;
    EXPECT_EQ(server_.setup({server, false, AF_INET, "", "", ""}), 0);
    EXPECT_EQ(client_.setup(context), 0);
    EXPECT_EQ(server_.start("test", t_manager, s_manager), 0);

    Datasource dt;
    dt.src.type = "endpoint";
    dt.src.input = "http://localhost2:2001/3";
    dt.dest.output = "http://localhost2:2001/3";
    dt.dest.directorypolicy = 0x1;
    dt.dest.emptypolicy = 0x0;

    Attr at;
    at.trigger = static_cast<uint32_t>(Trigger::manual);
    at.virtstorage = false;

    EXPECT_EQ(client_.create_deploy_task(1, dt, at, 0), static_cast<uint32_t>(Deploy_taskstatus::deployed));
    EXPECT_EQ(client_.create_deploy_task(1, dt, at, 0), -1);
    at.trigger = static_cast<uint32_t>(Trigger::create);
    EXPECT_EQ(client_.create_deploy_task(2, dt, at, 0), -1);
    s_manager.load_server_config("", "", 2222, false, "test", {"localhost", AF_INET, 7777, false, "", "", ""}, {"localhost", 9999});
    EXPECT_EQ(s_manager.start("test"), 0);
    s_manager.signin("test", "cluster", "localhost", 7777);
    EXPECT_EQ(client_.create_deploy_task(3, dt, at, 0), static_cast<uint32_t>(Deploy_taskstatus::triggered));
    EXPECT_NE(t_manager.get_task(1), nullptr);

    client.set_port(2002);
    client.set_url("localhost3");
    EXPECT_EQ(client_.create_terminate_task(1), 0);
    EXPECT_EQ(t_manager.get_task(1), nullptr);
}