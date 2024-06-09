#include"ATParser.h"
#include<gtest/gtest.h>
#include<string>
#include<vector>

using ATMessage = ::jxtd::misc::parser::at::ATMessage;
using ATParser = ::jxtd::misc::parser::at::ATParser;
using Parser = ::jxtd::misc::parser::Parser<ATMessage>;

bool result_compare(ATParser* at_parser ,ATMessage& expected_result);

TEST(TestATParser ,ParseCmdStr){
    ATParser* test1 = new ATParser("AT+CMD=first.=_1,second,third\r\n");
    ATParser* test2 = new ATParser("AT+VOID\r\n");
    ATParser test3("AT+VOID\\n"); 
    test3.parse();
    std::string cmd1 = (char*)"CMD";
    std::string cmd2 = (char*)"VOID";
    std::vector<std::string> param1;
    std::vector<std::string> param2;
    param1.push_back("first.=_1");
    param1.push_back("second");
    param1.push_back("third");
    std::string data1;
    std::string data2;
    ATMessage result1 = {CMD ,cmd1 ,param1 ,data1};
    ATMessage result2 = {CMD ,cmd2 ,param2 ,data2};
    EXPECT_TRUE(result_compare(test1 ,result1));
    EXPECT_TRUE(result_compare(test2 ,result2));
    EXPECT_FALSE(test3.is_correct());
    delete test1;
    delete test2;
}

TEST(TestATParser ,ParseCmdFile){
    char* file_contant1 = (char*)"AT+CMD=first,seco_nd,third\r\n";
    FILE* file1 = fmemopen((void*)file_contant1 ,strlen(file_contant1) ,"r");
    char* file_contant2 = (char*)"AT+VOID\r\n";
    FILE* file2 = fmemopen((void*)file_contant2 ,strlen(file_contant2) ,"r");
    char* file_contant3 = (char*)"ATVOID\r\n";
    FILE* file3 = fmemopen((void*)file_contant3 ,strlen(file_contant3) ,"r");
    char* file_contant4 = (char*)"AT+CMD=first,second,third\r\nabcdefg\0hi<<<\r\n";
    FILE* file4 = fmemopen((void*)file_contant4 ,strlen(file_contant4) + 8 ,"r");
    ATParser* test1 = new ATParser(file1);
    ATParser* test2 = new ATParser(file2);
    ATParser test3(file3);
    ATParser* test4 = new ATParser(file4);
    test3.parse();
    std::string cmd1 = (char*)"CMD";
    std::string cmd2 = (char*)"VOID";
    std::string cmd4 = (char*)"CMD";
    std::vector<std::string> param1;
    std::vector<std::string> param2;
    std::vector<std::string> param4;
    param1.push_back("first");
    param1.push_back("seco_nd");
    param1.push_back("third");
    param4.push_back("first");
    param4.push_back("second");
    param4.push_back("third");
    std::string data1;
    std::string data2;
    std::string data4 = "abcdefg";
    data4.push_back('\0');
    data4.append("hi");
    ATMessage result1 = {CMD ,cmd1 ,param1 ,data1};
    ATMessage result2 = {CMD ,cmd2 ,param2 ,data2};
    ATMessage result4 = {LONG ,cmd4 ,param4 ,data4};
    EXPECT_TRUE(result_compare(test1 ,result1));
    EXPECT_TRUE(result_compare(test2 ,result2));
    EXPECT_FALSE(test3.is_correct());
    EXPECT_TRUE(result_compare(test4 ,result4));
    delete test1;
    delete test2;
    delete test4;
}

TEST(TestATParser ,ParseSearchStr){
    ATParser* test1 = new ATParser("AT+SEARCH=?\r\n");
    ATParser* test2 = new ATParser("AT+SEARCHSECOND=?\r\n");
    ATParser test3("AT+SEARCHSECO!ND=?\r\n");
    test3.parse();
    std::string cmd1 = "SEARCH";
    std::string cmd2 = "SEARCHSECOND";
    std::vector<std::string> param1;
    std::vector<std::string> param2;
    std::string data1;
    std::string data2;
    ATMessage result1 = {SEARCH ,cmd1 ,param1 ,data1};
    ATMessage result2 = {SEARCH ,cmd2 ,param2 ,data2};
    EXPECT_TRUE(result_compare(test1 ,result1));
    EXPECT_TRUE(result_compare(test2 ,result2));
    EXPECT_FALSE(test3.is_correct());
    delete test1;
    delete test2;
}

TEST(TestATParser ,ParseTestStr){
    ATParser* test1 = new ATParser("AT+TEST?\r\n");
    ATParser* test2 = new ATParser("AT+TESTSECOND?\r\n");
    ATParser test3("AT+TaESTSECOND?\r\n");
    test3.parse();
    std::string cmd1 = "TEST";
    std::string cmd2 = "TESTSECOND";
    std::vector<std::string> param1;
    std::vector<std::string> param2;
    std::string data1;
    std::string data2;
    ATMessage result1 = {TEST ,cmd1 ,param1 ,data1};
    ATMessage result2 = {TEST ,cmd2 ,param2 ,data2};
    EXPECT_TRUE(result_compare(test1 ,result1));
    EXPECT_TRUE(result_compare(test2 ,result2));
    EXPECT_FALSE(test3.is_correct());
    delete test1;
    delete test2;
}

TEST(TestATParser ,ParseLongStr){
    std::string test1_stc = "AT+LONG=first,second,th_ird\r\nDataOne\r\nDataTwo\r\nDataThree\r\nDataF";
    test1_stc.push_back('\0');
    test1_stc.append("our\r\n<<<\r\n");
    ATParser* test1 = new ATParser(test1_stc);
    ATParser* test2 = new ATParser("AT+LONGSECOND=second,third,forth\r\nDataTwo\r\nDataThree\r\nDataFour\r\n<<<\r\n");
    ATParser* test3 = new ATParser("AT+LONGTHIRD\r\nDataThree\r\nDataFour\r\nDataFive\r\n@#$%^<<<\r\n");
    ATParser test4("AT+LONGSECOND=second?third,forth\r\nDataTwo\r\nDataThree\r\nDataFour\r\n<<<\r\n");
    test4.parse();
    std::string cmd1 = "LONG";
    std::string cmd2 = "LONGSECOND";
    std::string cmd3 = "LONGTHIRD";
    std::vector<std::string> param1;
    std::vector<std::string> param2;
    std::vector<std::string> param3;
    param1.push_back("first");
    param1.push_back("second");
    param1.push_back("th_ird");
    param2.push_back("second");
    param2.push_back("third");
    param2.push_back("forth");
    std::string data1 = "DataOne\r\nDataTwo\r\nDataThree\r\nDataF";
    data1.push_back('\0');
    data1.append("our\r\n");
    std::string data2 = "DataTwo\r\nDataThree\r\nDataFour\r\n";
    std::string data3 = "DataThree\r\nDataFour\r\nDataFive\r\n@#$%^";
    ATMessage result1 = {LONG ,cmd1 ,param1 ,data1};
    ATMessage result2 = {LONG ,cmd2 ,param2 ,data2};
    ATMessage result3 = {LONG ,cmd3 ,param3 ,data3};
    EXPECT_TRUE(result_compare(test1 ,result1));
    EXPECT_TRUE(result_compare(test2 ,result2));
    EXPECT_TRUE(result_compare(test3 ,result3));
    EXPECT_FALSE(test4.is_correct());
    delete test1;
    delete test2;
    delete test3;
}


bool result_compare(ATParser* at_parser ,ATMessage& expected_result){
    ATMessage* parsed_result = at_parser -> parse();
    EXPECT_TRUE(at_parser -> is_correct());
    EXPECT_TRUE(at_parser -> is_parsed());
    if(!parsed_result -> param.empty()){
        if(parsed_result -> param.size() != expected_result.param.size())
 	    return false;
	for(int i = 0; i < parsed_result -> param.size(); i++){
	    if(parsed_result -> param[i] != expected_result.param[i])
	        return false;
	}
    }
    if(!parsed_result -> data.empty())
        if(parsed_result -> data != expected_result.data)
	    return false;
    return (parsed_result -> type == expected_result.type)&&
	    (parsed_result -> cmd == expected_result.cmd);
}
