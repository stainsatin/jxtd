#include <gtest/gtest.h>
#include "Transforms.h"
#include "Compose.h"
#include"computation/infer/Tensor.h"
jxtd::computation::infer::Tensor setdt(const jxtd::computation::infer::Tensor&tensor)noexcept{
	char buf[25] = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0};
        jxtd::computation::infer::Tensor tmp(tensor);
        tmp.set_data((const void*)buf,(size_t)25);
        return tmp;
}


TEST(test_transform,transforms){
	char buf[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0
    };
    jxtd::computation::infer::Tensor tensor((const void*)buf,25);
    jxtd::misc::cv:: Transforms t("trans1");
    EXPECT_EQ("trans1", t.get_name());
    
    jxtd::misc::cv::Transforms::tran_ptr tptr = setdt;
    t.add_trans<jxtd::misc::cv::Transforms::tran_ptr>("do_tran1",tptr);
    t.start(t.get_output());
    auto result1 = t.get_output().get_data();
    for(int i = 0;i<25;i++)
      EXPECT_EQ(result1[i], 0);
}


TEST(test_transform,compose){
	jxtd::misc::cv:: Transforms t1("tran1");
	jxtd::misc::cv::Transforms::tran_ptr tptr = setdt;
    	t1.add_trans<jxtd::misc::cv::Transforms::tran_ptr>("do_tran1",tptr);
	jxtd::misc::cv:: Transforms t2("tran2");
	jxtd::misc::cv:: Transforms t3("tran3");
	jxtd::misc::cv:: Compose c;
	c.push_back(t1);
	c.push_back(t2);
	c.push_back(t3);
	EXPECT_EQ(c.size(),3);
	c.pop_back();
	c.pop_back();
	EXPECT_EQ(c.size(),1);
	char buf[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0
	};
	jxtd::computation::infer::Tensor tensor((const void*)buf,25);
	c(tensor);
	c.start(c.get_output());
	auto result = c.get_output().get_data();
	for(int i = 0;i<25;i++)
          EXPECT_EQ(result[i], 0);
}
