#include <gtest/gtest.h>
#include "DatatranServer.h"
#include <cstdint>
#include <mutex>
#include <filesystem>
#include <vector>
#include <utility>
#include <cstdlib>
#include <workflow/WFServer.h>
#include <workflow/WFFacilities.h>
#include "computation/datatransfer/client_task.h"

using DatatranServer = ::jxtd::storage::datatransfer::DatatranServer;
using DataClient = ::jxtd::computation::datatransfer::DataClient;
using DataClientSetupContext = ::jxtd::computation::datatransfer::DataSideContext;
using DataRequestContext = ::jxtd::computation::datatransfer::DataRequestContext;
using DataServerContext = ::jxtd::computation::datatransfer::DataSideContext;
using DataRequest = ::jxtd::proto::datatransfer::Request;
using DataResponse = ::jxtd::proto::datatransfer::Response;
using Datamanager = ::jxtd::storage::datamanagement::Datamanager;
using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;

TEST(testDatatrans, DatatranServer){
    std::filesystem::remove_all("./test");
    std::filesystem::remove_all("../Task_0");
    std::filesystem::remove_all("../Task_1");
    std::filesystem::remove_all("../Task_2");
    auto engineAttr_1 = std::make_pair("mid_engine_1", "name_engine_1");
    auto engineAttr_2 = std::make_pair("mid_engine_2", "name_engine_2");
    Adtsrc engine_1({0x1, 0x0, engineAttr_1, {}, "This is engine_1"});
    Adtsrc engine_2({0x2, 0x0, engineAttr_2, {}, "This is engine_2"});
    auto endpointAttr_1 = std::make_pair("proto_runtime_1", "url_runtime_1");
    auto endpointAttr_2 = std::make_pair("proto_runtime_2", "url_runtime_2");
    Adtsrc endpoint_1({0x1, 0x1, {}, endpointAttr_1, "This is runtime/endpoint_1"});
    Adtsrc endpoint_2({0x2, 0x1, {}, endpointAttr_2, "This is runtime/endpoint_2"});
    Adtsrc binary_1({0x1, 0x2, {}, {}, "This is binary_1"});
    Adtsrc binary_2({0x2, 0x2, {}, {}, "This is binary_2"});
    std::vector<Adtsrc> adtsrcs_1 = {engine_1, endpoint_1, binary_1};
    std::vector<Adtsrc> adtsrcs_2 = {engine_2, endpoint_2, binary_2};

    Datamanager adm;
    std::mutex mtx;
    adm.init("./test", true);
    DatatranServer server({
        AF_INET,
        "localhost",
        6666,
        false,
        "",
        "",
        "",
    });

    EXPECT_EQ(server.start(adm, mtx), 0);
    
    DataServerContext server1({
        "localhost",
        6666,
        false
    });
    DataClient client;
    DataClientSetupContext setup_ctx({
        "localhost",
        7777,
        false
    });//string_addr,int_port,bool_ssl

    DataRequestContext req_ctx_1({
        "src_client",
        "dest_client",
        "",
        0x1,
        adtsrcs_1
    });
    DataRequestContext req_ctx_3({
        "src_client",
        "dest_client",
        "",
        0x1,
        adtsrcs_2
    });

    DataRequestContext req_ctx_2({
        "src_client",
        "dest_client",
        "",
        0x0,
        {
            {0x1, 0x0, engineAttr_1, {}, },
            {0x1, 0x1, {}, endpointAttr_1, },
            {0x1, 0x2, {}, {}, }
        }//vector
    });
    DataRequestContext req_ctx_4({
        "src_client",
        "dest_client",
        "",
        0x0,
        {
            {0x0, 0x0, engineAttr_2, {}, },
            {0x2, 0x1, {}, endpointAttr_2, },
            {0x2, 0x2, {}, {}, }
        }//vector
    });

    std::vector<std::string> input_1;
    std::vector<std::pair<std::string, std::string>> master_mid_name_1;   
    std::vector<std::string> input_2;
    std::vector<std::pair<std::string, std::string>> master_mid_name_2;

    client.setup(setup_ctx);
    adm.task_bind(0x1, engineAttr_1, {});

    auto task_1 = client.create(server1, req_ctx_1, input_1, master_mid_name_1);
    WFFacilities::WaitGroup group_1(1);
    Workflow::start_series_work(task_1, [&group_1](const SeriesWork*){group_1.done();});
    group_1.wait();
    auto task_3 = client.create(server1, req_ctx_3, input_1, master_mid_name_1);
    WFFacilities::WaitGroup group_3(1);
    Workflow::start_series_work(task_3, [&group_3](const SeriesWork*){group_3.done();});
    group_3.wait();

    auto task_2 = client.create(server1, req_ctx_2, input_2, master_mid_name_2);
    WFFacilities::WaitGroup group_2(1);
    Workflow::start_series_work(task_2, [&group_2](const SeriesWork*){group_2.done();});
    group_2.wait();
    //检验task1
    std::string str_1 = input_2.back();
    EXPECT_EQ(str_1, "This is binary_1");
    auto pair_1 = master_mid_name_2.back();
    EXPECT_EQ(pair_1.first, engineAttr_1.first);
    EXPECT_EQ(pair_1.second, engineAttr_1.second);

    auto task_4 = client.create(server1, req_ctx_4, input_2, master_mid_name_2);
    WFFacilities::WaitGroup group_4(1);
    Workflow::start_series_work(task_4, [&group_4](const SeriesWork*){group_4.done();});
    group_4.wait();
    //检验task3
    std::string str_2 = input_2.back();
    EXPECT_EQ(str_2, "This is binary_1");
    auto pair_2 = master_mid_name_2.back();
    EXPECT_EQ(pair_2.first, engineAttr_2.first);
    EXPECT_EQ(pair_2.second, engineAttr_2.second);

    adm.deinit();
    std::filesystem::remove_all("./test");
    std::filesystem::remove_all("../Task_0");
    std::filesystem::remove_all("../Task_1");
    std::filesystem::remove_all("../Task_2");
    server.stop();
}