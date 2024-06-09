/**
 * test for client_task
 */
#include <gtest/gtest.h>
#include "proto/datatransfer/message.h"
#include "computation/datatransfer/client_task.cc"
#include "workflow/WFTaskFactory.h"
#include <string>
#include <filesystem>
#include <fstream>

using DataRequest = ::jxtd::proto::datatransfer::Request;
using DataResponse = ::jxtd::proto::datatransfer::Response;
using DataTask = WFNetworkTask<DataRequest, DataResponse>;
using Dtsrc = ::jxtd::proto::datatransfer::Dtsrc;
using Adtsrc = ::jxtd::proto::datatransfer::Adtsrc;

namespace jxtd::computation::datatransfer
{
    void resp_dispose(std::vector<std::pair<std::string, std::string>> &master_mid_name, const Dtsrc &dtsrc);
}

TEST(testDatatransfer, testcallback)
{
    std::vector<std::pair<std::string, std::string>> master_mid_name;
    std::string filepath = "../Task_" + std::to_string(0x1) + "/";
    std::filesystem::remove_all(filepath);
    Adtsrc a;
    a.taskid = 0x1;
    a.binary = "shape";
    a.engineattr.first = "123";
    a.engineattr.second = "456";
    Dtsrc dtsrc;
    dtsrc.master = a;
    Adtsrc b;
    b.binary = "34525423543";
    dtsrc.deps.push_back(std::make_pair(b, "a/b/c/d"));
    jxtd::computation::datatransfer::resp_dispose(master_mid_name, dtsrc);
    std::ifstream infile;
    infile.open((filepath + "JXTDENGINE_123_456.so").c_str(), std::ios::in | std::ios::binary);

    EXPECT_EQ(infile.is_open(), true);
    infile.seekg(0, std::ios::end);
    std::streamsize fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::string file(fileSize, '\0');
    infile.read(&file[0], fileSize);
    EXPECT_EQ(file, "shape");
    infile.close();
    infile.open((filepath + "a/b/c/d").c_str(), std::ios::in | std::ios::binary);
    EXPECT_EQ(infile.is_open(), true);
    infile.seekg(0, std::ios::end);
    fileSize = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::string file_(fileSize, '\0');
    infile.read(&file_[0], fileSize);
    EXPECT_EQ(file_, "34525423543");
    std::filesystem::remove_all(filepath);
}