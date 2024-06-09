#include "ImagePrc.h"
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

TEST(test_cv_internal_base, rectangle){
    char buf[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0
    };
    cv::Mat m(5, 5, CV_8UC1, (void*)buf);
    auto oob = m.clone();
    auto backup = oob.clone();
    auto result = jxtd::misc::cv::internal::rectangle(&m, 1, 1, 3, 3);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result, &m);
    EXPECT_TRUE(m.isContinuous());
    for(int i = 0;i<9 ;i++)
        EXPECT_EQ(m.data[i], 1);

    auto same = jxtd::misc::cv::internal::rectangle(&oob, 0, 0, 5, 5);
    ASSERT_EQ(backup.rows, same -> rows);
    ASSERT_EQ(backup.cols, same -> cols);
    EXPECT_EQ(nullptr,
                jxtd::misc::cv::internal::rectangle(&oob, 1, 1, 5, 5));
}

TEST(test_cv_internal_base, rotate){
    char buf[20] = {
        0, 1, 2, 3, 4,
        5, 6, 7, 8, 9,
        10, 11, 12, 13, 14,
        15, 16, 17, 18, 19
    };
    char buf27[20] = {
        15, 10, 5, 0,
        16, 11, 6, 1,
        17, 12, 7, 2,
        18, 13, 8, 3,
        19, 14, 9, 4
    };
    char buf18[20] = {
        19, 18, 17, 16, 15,
        14, 13, 12, 11, 10,
        9, 8, 7, 6, 5,
        4, 3, 2, 1, 0
    };
    char buf9[20] = {
        4, 9, 14, 19,
        3, 8, 13, 18,
        2, 7, 12, 17,
        1, 6, 11, 16,
        0, 5, 10, 15
    };
    cv::Mat raw(4, 5, CV_8UC1, (void*)buf);
    cv::Mat r9(5, 4, CV_8UC1, (void*)buf9);
    cv::Mat r18(4, 5, CV_8UC1, (void*)buf18);
    cv::Mat r27(5, 4, CV_8UC1, (void*)buf27);

    //now common rotate
    auto t9 = raw.clone();
    auto result = jxtd::misc::cv::internal::rotate(&t9, 90);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result, &t9);
    EXPECT_EQ(r9.rows, t9.rows);
    EXPECT_EQ(r9.cols, t9.cols);
    auto t18 = raw.clone();
    result = jxtd::misc::cv::internal::rotate(&t18, 180);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result, &t18);
    EXPECT_EQ(r18.rows, t18.rows);
    auto t27 = raw.clone();
    result = jxtd::misc::cv::internal::rotate(&t27, 270);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result, &t27);
    EXPECT_EQ(r27.cols, t27.cols);
    auto tn9 = raw.clone();
    result = jxtd::misc::cv::internal::rotate(&tn9, -90);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result, &tn9);
    EXPECT_EQ(r27.rows, tn9.rows);
    EXPECT_EQ(r27.cols, tn9.cols);

    //now overlap rotate
    auto t54 = raw.clone();
    result = jxtd::misc::cv::internal::rotate(&t54, 540);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result, &t54);
    ASSERT_EQ(t54.cols, r18.cols);
    ASSERT_EQ(t54.rows, r18.rows);
    for(int i = 0;i < t54.cols*r18.rows;i++)
	EXPECT_EQ(t54.data[i], r18.data[i]);


    //final abnormal rotate
    auto tnalign = raw.clone();
    EXPECT_EQ(nullptr,
                jxtd::misc::cv::internal::rotate(&tnalign, 160));
}
//TODO:other func
