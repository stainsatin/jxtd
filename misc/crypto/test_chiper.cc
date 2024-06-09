#pragma warning(disable:4996)
#include"Chiper.h"
#include"DigestChiper.h"
#include"RandomGenerator.h"
#include"SymmeticChiper.h"
#include"AsymmeticChiper.h"
#include<gtest/gtest.h>
#include<string>
TEST(test_chiper,digest){
	jxtd::misc::crypto::Md5_Digest ch1;
    std:: string s1 = "abcdefghijklmnop";
    std:: string s2 = "abcdefghijklmnop";
    std:: string s3 = "bbcdefghijklmnop";
    std::string res = ch1.digest(s1);
    std::string res2 = ch1.digest(s2);
    std::string res3 = ch1.digest(s3);
    EXPECT_EQ(res.length(),res2.length());
    for(int i=0;i<res.length();i++){
        EXPECT_EQ(res[i],res2[i]);
    }
    EXPECT_STRNE(res.c_str(),res3.c_str());

    jxtd::misc::crypto::SHA1_Digest ch2;
    res = ch2.digest(s1);
    res2 = ch2.digest(s2);
    res3 = ch2.digest(s3);
    EXPECT_STREQ(res.c_str(),res2.c_str());
    EXPECT_STRNE(res.c_str(),res3.c_str());

    jxtd::misc::crypto::SM3_Digest ch3;
    res = ch3.digest(s1);
    res2 = ch3.digest(s2);
    res3 = ch3.digest(s3);
    EXPECT_STREQ(res.c_str(),res2.c_str());
    EXPECT_STRNE(res.c_str(),res3.c_str());
}

TEST(test_chiper,sym){
    jxtd::misc::crypto::DRBG_Random gen;
    gen.srand("abcdefghijklmnop");
    jxtd::misc::crypto::AES128_Chiper ch1(&gen);
    std::string s1 = "1234567890123456";
    std::string tmp = ch1.encrypt(s1,jxtd::misc::crypto::SymmeticChiper::CBC_Mode);
    std::string res = ch1.decrypt(tmp,jxtd::misc::crypto::SymmeticChiper::CBC_Mode);
    EXPECT_STREQ(s1.c_str(),res.c_str());

    jxtd::misc::crypto::AES128_Chiper ch2(&gen);
    std::string tmp1 = ch2.encrypt(s1,jxtd::misc::crypto::SymmeticChiper::ECB_Mode);
    std::string res1 = ch2.decrypt(tmp1,jxtd::misc::crypto::SymmeticChiper::ECB_Mode);
    EXPECT_STREQ(s1.c_str(),res1.c_str());

    jxtd::misc::crypto::SM4_Chiper ch3(&gen);
    std::string tmp3 = ch3.encrypt(s1,jxtd::misc::crypto::SymmeticChiper::CFB_Mode);
    std::string res3 = ch3.decrypt(tmp3,jxtd::misc::crypto::SymmeticChiper::CFB_Mode);
    EXPECT_STREQ(s1.c_str(),res3.c_str());
}

TEST(test_chiper,Asy){
    jxtd::misc::crypto::DRBG_Random gen;
    gen.srand("abcdefghijklmnop");
    jxtd::misc::crypto::RSA_Chiper ch1(&gen);   
    std::string s1 = "1234567890123456";
    std::string tmp = ch1.encrypt(s1,true);
    std::string res = ch1.decrypt(tmp,false);
    EXPECT_STREQ(s1.c_str(),res.c_str());
}
