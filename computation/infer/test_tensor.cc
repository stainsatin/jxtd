#include <gtest/gtest.h>
#include"Tensor.h"

TEST(test_tensor_base, get_data){
	char buf[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0
    };
    jxtd::computation::infer::Tensor t((const void*)buf,25);
    const int Type = 1;
    auto result = t.get_data();
    for(int i = 0;i<25;i++)
      EXPECT_EQ(result[i], buf[i]);
    
    char buf1[10]={
    0,0,0,0,0,
    1,1,1,1,1
    };
    
    t.set_data((const void*)buf1,10);
    auto result1 = t.get_data();
    for(int i = 0;i<10;i++)
      EXPECT_EQ(result1[i], buf1[i]);
}
