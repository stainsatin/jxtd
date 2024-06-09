#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include "management/taskdep/taskdep.h"
#include "proto/taskdep/message.h"
#include <string>

using Endpoint = ::jxtd::proto::taskdep::Endpoint;
using Request = ::jxtd::proto::taskdep::Request;
using Response = ::jxtd::proto::taskdep::Response;
using TaskdepServerContext = Endpoint;
using TaskdepSetupContext = ::jxtd::management::taskdep::TaskdepClientContext;
using Engine = Request::Engine;
using Datasource = Request::Datasource;
using Attr = Request::Attr;
using ManagementClient = ::jxtd::management::taskdep::ManagementClient;

TEST(testTaskdep, testtaskdep)
{
    ManagementClient client;
    TaskdepSetupContext setup_;
    setup_.management.set_proto("file");
    setup_.management.set_url("1");
    setup_.management.set_port(11);
    setup_.management.set_path("/111");
    setup_.management.set_authtype(0x0);
    setup_.ssl = false;
    EXPECT_EQ(client.setup(setup_), 0);
    Engine engine;
    engine.binary = "1";
    engine.id = "2";
    engine.name = "3";
    engine.deps.emplace_back(engine);
    EXPECT_EQ(client.create_neogiate_task(10, engine), 0);
}