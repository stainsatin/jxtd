#include<gtest/gtest.h>
#include <workflow/WFFacilities.h>
#include"infer.h"
#include<string>
#include<iostream>
#include"SubmitData.pb.h"

using SubmitRequest = ::jxtd::computation::infer::SubmitRequest;
using SubmitResponse = ::jxtd::computation::infer::SubmitResponse;
using DrccpServerManager = ::jxtd::computation::infer::DrccpServerManager;
using Taskmanager = ::jxtd::computation::infer::Taskmanager;

bool test_SubmitRequest(uint32_t&& taskid ,std::string&& input_path ,std::string&& output_path ,uint32_t&& period);

bool test_SubmitResponse(uint32_t&& taskid ,uint32_t&& times ,std::string&& result);

bool test_DrccpServerManager(std::string& name ,SubmitRequest& req);

TEST(testCase ,SubmitRequest){
    EXPECT_TRUE(test_SubmitRequest(2222 ,"input_path" ,"output_path" ,3333));	
}

TEST(testCase ,SubmitResponse){
    EXPECT_TRUE(test_SubmitResponse(4444 ,5555 ,"result"));
}

TEST(testCase ,DrccpServerManager){
    std::string test_string = "here's my name";
    uint32_t taskid = 7777;
    std::string input_path = "input_path_1";
    std::string output_path = "output_path_1";
    uint32_t period = 8888;
    SubmitRequest req;
    req.set_taskid(taskid);
    req.set_input(input_path, "");
    req.set_output(output_path, 0x0, 0x0);
    req.set_period(period);
    EXPECT_TRUE(test_DrccpServerManager(test_string ,req));
}

bool test_SubmitRequest(uint32_t&& taskid ,std::string&& input_path ,std::string&& output_path ,uint32_t&& period){
    SubmitRequest test1;
    SubmitRequest test2;
    test1.set_taskid(taskid);
    test1.set_input(input_path, "");
    test1.set_output(output_path, 0x0, 0x0);
    test1.set_period(period);
    test2.parse(test1.dump());
    return (test1.get_taskid() == taskid)&
           (test1.get_input_path() == input_path)&
	   (test1.get_output_path() == output_path)&
       (test1.get_directory_policy() == 0x0)&
       (test1.get_empty_policy() == 0x0)&
	   (test1.get_period() == period)&
           (test2.get_taskid() == taskid)&
           (test2.get_input_path() == input_path)&
	   (test2.get_output_path() == output_path)&
       (test2.get_directory_policy() == 0x0)&
       (test2.get_empty_policy() == 0x0)&
	   (test2.get_period() == period);
}

bool test_SubmitResponse(uint32_t&& taskid ,uint32_t&& times ,std::string&& result){
    SubmitResponse test1;
    SubmitResponse test2;
    test1.set_taskid(taskid);
    test1.set_times(times);
    test1.set_result(result);
    test1.set_output("test", 0x0, 0x0);
    test2.parse(test1.dump());
    return (test1.get_taskid() == taskid)&
	   (test1.get_times() == times)&
       (test1.get_output_path() == "test")&
       (test1.get_directory_policy() == 0x0)&
       (test1.get_empty_policy() == 0x0)&
	   (test1.get_result() == result)&
           (test2.get_taskid() == taskid)&
	   (test2.get_times() == times)&
       (test2.get_output_path() == "test")&
       (test2.get_directory_policy() == 0x0)&
       (test2.get_empty_policy() == 0x0)&
	   (test2.get_result() == result);
}

bool test_DrccpServerManager(std::string& name ,SubmitRequest& req){
    DrccpServerManager manager_;
    int i;

    manager_.load_server_config("", "", 2222, false, name ,
		              { "localhost", AF_INET, 7777,false,"", "", ""} ,
			      {"localhost" ,9999});
    for(i = 9000;i < 9010 ;++i){
	std::string disturbance_name = std::to_string(i);
        manager_.load_server_config("", "", 3333, false, disturbance_name ,
			          { "localhost", AF_INET, i, false,"", "", ""} ,
				  {"localhost" ,1234});
    }
    manager_.start(name);
    manager_.signin(name ,"cluster" ,"localhost" ,7777);
    Taskmanager manager;
    auto response = manager_.submit(name, req, manager);
    if(response == nullptr)
        return false;
    manager.deploy_task(1, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(2, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(5, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(6, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(3, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(7, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(8, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.deploy_task(4, "1", "1", "", 2, 2, 2, 0, 0, {});
    manager.add_deps(1, {2, 3, 8});
    manager.add_deps(2, {3, 7 ,8});
    manager.add_deps(6, {1, 2, 4, 6, 8, 5});
    EXPECT_EQ(manager.submit(manager_, name, 1), nullptr);
    manager.remove_task(1, true);
    manager.remove_task(8, false);
    EXPECT_EQ(manager.submit(manager_, name, 8), nullptr);
    EXPECT_NE(manager.submit(manager_, name, 7), nullptr);
    return true;
}	
