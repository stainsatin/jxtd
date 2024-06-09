#include <gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include <string>
#include "proto/taskdep/message.h"
#include "management/taskdep/taskdep.h"
#include "management/inferprovider/task.h"
#include "router/lib/cmd.h"
#include "router/lib/flag/flag.h"
#include "misc/parser/at/ATParser.h"
#include <unordered_map>
#include <functional>
#include <cstring>
#include <vector>
#include <string.h>
#include <fstream>
#include <filesystem>
#include "computation/taskdep/taskdep.h"
#include "storage/taskdep/taskdep.h"

using Taskresult = ::jxtd::router::Taskresult;
using Taskmanager = ::jxtd::management::inferprovider::Task_Manager;
using Method = ::jxtd::management::inferprovider::Trigger_method;
using Task = ::jxtd::management::inferprovider::Task;
using Taskdepmanagement = ::jxtd::management::taskdep::ManagementClient;
using Endpoint = ::jxtd::proto::taskdep::Endpoint;
using ATMessage = ::jxtd::misc::parser::at::ATMessage;
using ATParser = ::jxtd::misc::parser::at::ATParser;
using Flag = ::jxtd::router::flag::Flag;
using Cluster_Server = ::jxtd::computation::taskdep::ClusterServer;
using Task_manager = ::jxtd::computation::infer::Taskmanager;
using Servermanager = ::jxtd::computation::infer::DrccpServerManager;
using StorageServer = ::jxtd::storage::taskdep::StorageServer;
using Datamanager = ::jxtd::storage::datamanagement::Datamanager;

Taskresult &task = ::jxtd::router::task;
Taskdepmanagement &management_client = ::jxtd::router::management;
Taskmanager &manager = ::jxtd::router::manager;
Flag &flag = ::jxtd::router::flag_;

TEST(testCMD, testcmd)
{
    std::filesystem::remove_all("../name");
    std::filesystem::remove_all("./test");
    {
        ATParser parser1("AT+REG=name=sss,adf\r\n");
        auto message1 = parser1.parse();
        EXPECT_EQ(::jxtd::router::REG(*message1), "AT+ENVAL");

        ATParser parser2("AT+REG?\r\n");
        auto message2 = parser2.parse();
        EXPECT_EQ(::jxtd::router::REG(*message2), "AT+ERR=-2,name is not registered");

        ATParser parser3("AT+REG=name\r\n");
        auto message3 = parser3.parse();
        EXPECT_EQ(::jxtd::router::REG(*message3), "AT+OK");

        ATParser parser4("AT+REG=name\r\n");
        auto message4 = parser4.parse();
        EXPECT_EQ(::jxtd::router::REG(*message4), "AT+ERR=0,name is registered");
        EXPECT_EQ(std::filesystem::exists("../name"), true);

        ATParser parser5("AT+REG?\r\n");
        auto message5 = parser5.parse();
        EXPECT_EQ(::jxtd::router::REG(*message5), "AT+OK");

        ATParser parser6("AT+REG=x\r\n");
        auto message6 = parser6.parse();
        EXPECT_EQ(::jxtd::router::REG(*message6), "AT+OK");
        EXPECT_EQ(std::filesystem::exists("../name"), false);
        EXPECT_EQ(std::filesystem::exists("../x"), true);
    }

    {
        ATParser parser1("AT+RTUNREG\r\n");
        auto message1 = parser1.parse();
        EXPECT_EQ(::jxtd::router::RTUNREG(*message1), "AT+OK");

        ATParser parser2("AT+RTUNREG\r\n");
        auto message2 = parser2.parse();
        EXPECT_EQ(::jxtd::router::RTUNREG(*message2), "AT+ERR");

        ATParser parser3("AT+RTUNREG=NAME\r\n");
        auto message3 = parser3.parse();
        EXPECT_EQ(::jxtd::router::RTUNREG(*message3), "AT+ENVAL");
    }

    {
        ATParser parser1("AT+RTBUSY?\r\n");
        EXPECT_EQ(::jxtd::router::RTBUSY(*parser1.parse()), "AT+IDLE");

        ATParser parser2("AT+RTBUSY?=\r\n");
        EXPECT_EQ(::jxtd::router::RTBUSY(*parser2.parse()), "AT+ENVAL");
    }

    std::string mid;
    
    {
        ATParser parser1("AT+MDDECL=name\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser1.parse()), "AT+MDINFO=-4,the service is not registered");

        ATParser parser("AT+REG=name\r\n");
        EXPECT_EQ(::jxtd::router::REG(*parser.parse()), "AT+OK");

        ATParser parser2("AT+MDDECL=sss,rrr\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser2.parse()), "AT+MDINFO=0,sss,rrr");

        ATParser parser3("AT+MDDECL=sss,bbb\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser3.parse()), "AT+MDINFO=0,sss,rrr,remote model has existed");

        ATParser parser_31("AT+MDDECL=xxx,rrr\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser_31.parse()), "AT+MDINFO=0,sss,rrr,remote model has existed");

        ATParser parser_32("AT+MDDECL=xxx,bbb\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser_32.parse()), "AT+MDINFO=0,xxx,bbb");
        // cloud -> xxx,bbb 和 sss,rrr

        ATParser parser4("AT+MDDECL=xxx,aaaa\r\n123456789<<<\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser4.parse()), "AT+ENVAL");

        ATParser parser5("AT+MDDECL=lll\r\n123456789<<<\r\n");
        mid = ::jxtd::router::MDDECL(*parser5.parse());
        EXPECT_EQ(mid.compare(0,16,"AT+MDINFO=0,lll,",0,16), 0);
        size_t pos = mid.find_last_of(",");
        mid.erase(0,pos+1);
        EXPECT_EQ(std::filesystem::exists("../name/" + mid), true);
        // private -> lll,id
        
        std::ifstream infile;
        infile.open("../name/" + mid, std::ios::in | std::ios::binary);
        infile.seekg(0, std::ios::end);
        auto fileSize = infile.tellg();
        infile.seekg(0, std::ios::beg);
        std::string data(fileSize, '\0');
        infile.read(&data[0], fileSize);
        infile.close();

        EXPECT_EQ(data, "123456789");

        ATParser parser6("AT+MDDECL=name_aaa\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser6.parse()), "AT+MDINFO=-1,the model is not existed");

        ATParser parser7("AT+MDDECL=name_sss\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser7.parse()), "AT+MDINFO=0,sss,rrr");

        ATParser parser8("AT+MDDECL=id_" + mid + "\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser8.parse()), "AT+MDINFO=0,lll," + mid);

        ATParser parser9("AT+MDDECL=?\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser9.parse()), "AT+MDINFO=(0,lll," + mid + "),(0,sss,rrr),(0,xxx,bbb),");
    }

    {
        ATParser parser1("AT+MDRM=name_xxx\r\n");
        EXPECT_EQ(::jxtd::router::MDRM(*parser1.parse()), "AT+MDMOD=0");

        ATParser parser2("AT+MDRM=mid_rrr\r\n");
        EXPECT_EQ(::jxtd::router::MDRM(*parser2.parse()), "AT+MDMOD=0");

        ATParser parser3("AT+MDRM=id_" + mid + "\r\n");
        EXPECT_EQ(::jxtd::router::MDRM(*parser3.parse()), "AT+MDMOD=0");
        EXPECT_EQ(std::filesystem::exists("../name/" + mid), false);

        ATParser parser4("AT+MDRM=name_xxx\r\n");
        EXPECT_EQ(::jxtd::router::MDRM(*parser4.parse()), "AT+MDMOD=-1,model is not existed");
        // cloud -> xxx,bbb 和 sss,rrr
        // private -> lll,mid
    }

    {
        ATParser parser2("AT+MDDECL=sss,rrr\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser2.parse()), "AT+MDINFO=0,sss,rrr");

        ATParser parser3("AT+MDDECL=sss,bbb\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser3.parse()), "AT+MDINFO=0,sss,rrr,remote model has existed");

        ATParser parser_31("AT+MDDECL=xxx,rrr\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser_31.parse()), "AT+MDINFO=0,sss,rrr,remote model has existed");

        ATParser parser_32("AT+MDDECL=xxx,bbb\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser_32.parse()), "AT+MDINFO=0,xxx,bbb");
        // cloud -> xxx,bbb 和 sss,rrr

        ATParser parser4("AT+MDDECL=xxx,aaaa\r\n123456789<<<\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser4.parse()), "AT+ENVAL");

        ATParser parser5("AT+MDDECL=lll\r\n123456789<<<\r\n");
        mid = ::jxtd::router::MDDECL(*parser5.parse());
        EXPECT_EQ(mid.compare(0,16,"AT+MDINFO=0,lll,",0,16), 0);
        size_t pos = mid.find_last_of(",");
        mid.erase(0,pos+1);
        EXPECT_EQ(std::filesystem::exists("../name/" + mid), true);
        // private -> lll,id
        
        std::ifstream infile;
        infile.open("../name/" + mid, std::ios::in | std::ios::binary);
        infile.seekg(0, std::ios::end);
        auto fileSize = infile.tellg();
        infile.seekg(0, std::ios::beg);
        std::string data(fileSize, '\0');
        infile.read(&data[0], fileSize);
        infile.close();

        EXPECT_EQ(data, "123456789");

        ATParser parser6("AT+MDDECL=name_aaa\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser6.parse()), "AT+MDINFO=-1,the model is not existed");

        ATParser parser7("AT+MDDECL=name_sss\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser7.parse()), "AT+MDINFO=0,sss,rrr");

        ATParser parser8("AT+MDDECL=id_" + mid + "\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser8.parse()), "AT+MDINFO=0,lll," + mid);

        ATParser parser9("AT+MDDECL=?\r\n");
        EXPECT_EQ(::jxtd::router::MDDECL(*parser9.parse()), "AT+MDINFO=(0,lll," + mid + "),(0,sss,rrr),(0,xxx,bbb),");
    }

    {
        // cloud -> xxx,bbb 和 sss,rrr
        // private -> lll,mid
        flag.datatransfer_router = "localhost2";
        ATParser parser1("AT+MDPUSH=name_lll\r\n");
        EXPECT_EQ(::jxtd::router::MDPUSH(*parser1.parse()), "AT+MDPUSH=-5,the source url is incorrect");

        flag.datatransfer_router = "localhost2:3333";
        ATParser parser2("AT+MDPUSH=name_lll\r\n");
        EXPECT_EQ(::jxtd::router::MDPUSH(*parser2.parse()), "AT+MDPUSH=-5,the dest url is incorrect");

        flag.datatransfer_storage = "localhost3:4444";
        ATParser parser3("AT+MDPUSH=name_aaa\r\n");
        EXPECT_EQ(::jxtd::router::MDPUSH(*parser3.parse()), "AT+MDPUSH=-1,aaa,the model is not existed");

        // ATParser parser4("AT+MDPUSH=name_lll\r\n");
        // EXPECT_EQ(::jxtd::router::MDPUSH(*parser4.parse()), "AT+MDPUSH=-5,unable to push model");
        // EXPECT_EQ(std::filesystem::exists("../name/" + mid), true);

        ATParser parser5("AT+MDPUSH=xxx\r\n1237890<<<\r\n");
        EXPECT_EQ(::jxtd::router::MDPUSH(*parser5.parse()), "AT+MDPUSH=-2,xxx,bbb,duplicate engine indexes");
    }

    Endpoint storage, computation, management;
    storage.set_proto("jxtd");
    computation.set_proto("jxtd");
    management.set_proto("jxtd");
    storage.set_url("localhost");
    computation.set_url("localhost");
    management.set_url("localhost");
    storage.set_port(2001);
    computation.set_port(2002);
    management.set_port(2003);
    storage.set_path("1");
    computation.set_path("2");
    management.set_path("3");
    storage.set_authtype(0x0);
    computation.set_authtype(0x0);
    management.set_authtype(0x0);

    Cluster_Server computation_server;
    Task_manager t_manager;
    Servermanager s_manager;
    StorageServer storage_server;

    task.start();
    management_client.setup({management, computation, storage, "", false});
    computation_server.setup({computation, false, AF_INET, "", "", ""});
    t_manager.set_src(computation, false, "");
    t_manager.set_dest(storage);
    EXPECT_EQ(computation_server.start("test", t_manager, s_manager), 0);

    s_manager.load_server_config("", "", 6666, false, "test", {"localhost", AF_INET, 7777, false, "", "", ""}, {"localhost", 9999});
    EXPECT_EQ(s_manager.start("test"), 0);
    s_manager.signin("test", "cluster", "localhost", 7777);

    Datamanager manager;
    std::mutex task_mutex;
    EXPECT_EQ(manager.init("./test", true), 0);
    EXPECT_EQ(storage_server.setup({storage, false, AF_INET, "", "", ""}), 0);
    EXPECT_EQ(storage_server.start(manager, task_mutex), 0);

    {
        ATParser parser1("AT+CPMOD\r\n");
        EXPECT_EQ(::jxtd::router::CPMOD(*parser1.parse()), "AT+CPSTA=cancel=true");

        ATParser parser2("AT+CPMOD?\r\n");
        EXPECT_EQ(::jxtd::router::CPMOD(*parser2.parse()), "AT+CPSTA=cancel=true");
    }

    {
        // cloud -> xxx,bbb 和 sss,rrr
        // private -> lll,mid
        ATParser parser1("AT+CPSTART=id_SS,data\r\n123456<<<\r\n");
        EXPECT_EQ(::jxtd::router::CPSTART(*parser1.parse()), "AT+CPRES=-1,the model is not existed");

        ATParser parser2("AT+CPSTART=id_" + mid + ",data\r\n123456<<<\r\n");
        EXPECT_EQ(::jxtd::router::CPSTART(*parser2.parse()), "AT+CPRES=-8,unable to get result");

        ATParser parser3("AT+CPVAL=5\r\n");
        EXPECT_EQ(::jxtd::router::CPVAL(*parser3.parse()), "AT+CPRES=-7,the task is canceled");

        ATParser parser4("AT+CPSTART=id_" + mid + ",data,nonblock\r\n123456<<<\r\n");
        EXPECT_EQ(::jxtd::router::CPSTART(*parser4.parse()), "AT+CPRES=0,1");
    }

    {
        ATParser parser1("AT+CPMOD?\r\n");
        EXPECT_EQ(::jxtd::router::CPMOD(*parser1.parse()), "AT+CPSTA=cancel=false");

        ATParser parser2("AT+CPRDY?\r\n");
        EXPECT_EQ(::jxtd::router::CPRDY(*parser2.parse()), "AT+BUSY");

        ATParser parser3("AT+CPMOD\r\n");
        EXPECT_EQ(::jxtd::router::CPMOD(*parser3.parse()), "AT+CPSTA=cancel=true");

        ATParser parser4("AT+CPMOD?\r\n");
        EXPECT_EQ(::jxtd::router::CPMOD(*parser4.parse()), "AT+CPSTA=cancel=true");
    }

    {
        ATParser parser1("AT+CPRDY?\r\n");
        EXPECT_EQ(::jxtd::router::CPRDY(*parser1.parse()), "AT+READY");
    }

    {
        ATParser parser1("AT+CPVAL\r\n");
        EXPECT_EQ(::jxtd::router::CPVAL(*parser1.parse()), "AT+CPRES=-7,the task is canceled");

        ATParser parser2("AT+CPVAL\r\n");
        EXPECT_EQ(::jxtd::router::CPVAL(*parser2.parse()), "AT+CPRES=-7,the task is canceled");

        ATParser parser3("AT+CPVAL=1\r\n");
        EXPECT_EQ(::jxtd::router::CPVAL(*parser3.parse()), "AT+CPRES=-7,the task is canceled");

        ATParser parser4("AT+CPVAL=1,2000\r\n");
        EXPECT_EQ(::jxtd::router::CPVAL(*parser4.parse()), "AT+CPRES=-7,the task is canceled");
    }

    {
        ATParser parser1("AT+TRANS=1\r\nls -l<<<\r\n");
        std::string out = ::jxtd::router::TRANS(*parser1.parse());
        EXPECT_FALSE(out.empty());

        ATParser parser2("AT+STRANS=2\r\nls -l<<<\r\n");
        EXPECT_EQ(::jxtd::router::STRANS(*parser2.parse()), "Wrong sudo command");
    }

    storage_server.stop();
    task.stop();
    s_manager.stop("test");
    
    ATParser parser_end("AT+RTUNREG\r\n");
    auto message_end = parser_end.parse();
    EXPECT_EQ(::jxtd::router::RTUNREG(*message_end), "AT+OK");
    manager.deinit();
}