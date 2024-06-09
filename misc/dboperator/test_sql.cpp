#include<vector>
#include<variant>
#include"sqliteUtil.h"
#include"sqliteConnector.h"
#include<gtest/gtest.h>
#include<string>
TEST(test_sql,test1){
    std::string sql = "example.db";
	jxtd::misc::dboperator::sqliteUtil& db = jxtd::misc::dboperator::sqliteUtil::getInstance();
	std::unique_ptr<jxtd::misc::dboperator::sqliteConnector> con = db.open(sql);
    std::string sql1 = "CREATE TABLE PEOPLE (ID INTEGER PRIMARY KEY, Name TEXT, Age INTEGER);"; 
    con->excuteQuery(sql1);
    con->beginTranscation();
    std::string sql2 = "INSERT INTO PEOPLE (ID, Name, Age) VALUES (1, 'Alice', 20);"; 
    int rc = con->excuteQuery(sql2);
    EXPECT_EQ(1,rc);
    std::string sql3 = "INSERT INTO PEOPLE (ID, Name, Age) VALUES (2, 'Bob', 20);"; 
    rc = con->excuteQuery(sql3);
    if(rc==1)
        con->commitTranscation();
    else
        con->rollbackTranscation();
    std::string sql4 = "SELECT * FROM PEOPLE;";
    std::vector<std::tuple<int, std::string, int>> res;
    res = con->getResultSet<int, std::string, int>(sql4);
    EXPECT_EQ(2,res.size());
}
