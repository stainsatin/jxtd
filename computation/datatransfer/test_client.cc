/**
 * test for client_task
 */
#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFServer.h>
#include <workflow/Workflow.h>
#include "computation/datatransfer/client_task.h"
#include "proto/datatransfer/message.h"
#include <string>

using DataClient = ::jxtd::computation::datatransfer::DataClient;
using DataClientSetupContext = ::jxtd::computation::datatransfer::DataSideContext;
using DataRequestContext = ::jxtd::computation::datatransfer::DataRequestContext;
using DataServerContext = ::jxtd::computation::datatransfer::DataSideContext;
using DataRequest = ::jxtd::proto::datatransfer::Request;
using DataResponse = ::jxtd::proto::datatransfer::Response;

TEST(testDatatransfer, testclient)
{
    std::vector<std::string> input;
    std::vector<std::pair<std::string, std::string>> master_mid_name;
    DataClientSetupContext setup_;
    setup_.addr = "sss";
    setup_.port = 2222;
    setup_.ssl = false;
    DataClient client;
    client.setup(setup_);
    EXPECT_EQ(client.get_client()->addr, "sss");
    EXPECT_EQ(client.get_client()->port, 2222);
    EXPECT_EQ(client.get_client()->ssl, false);

    DataRequestContext req_ctx;
    req_ctx.src = req_ctx.dest = req_ctx.token = "1";
    req_ctx.direction = 0x1;

    DataServerContext server;
    server.addr = "ddd";
    server.port = 3333;

    auto task = client.create(server, req_ctx, input, master_mid_name);
    EXPECT_NE(task, nullptr);
    // WFFacilities::WaitGroup group(1);
    // Workflow::start_series_work(task, [&group](const SeriesWork*){group.done();});
    // group.wait();
}
