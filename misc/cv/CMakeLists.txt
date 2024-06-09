add_subdirectory(internal)
set(SRC Transforms.cc Compose.cc )
add_library(transform OBJECT)
target_sources(transform PRIVATE  ${SRC})
add_dependencies(transform opencv-4.6.0)

add_executable(test_trans)
target_sources(test_trans PRIVATE test_trans.cc)
add_dependencies(test_trans gtest-1.12.0 opencv-4.6.0)
target_link_directories(test_trans PRIVATE ${PROJECT_SOURCE_DIR}/computation/infer internal)
target_link_libraries(test_trans PRIVATE infer transform misc_cv_internal gtest_main gtest opencv_imgproc opencv_core 
opencv_imgcodecs dl pthread z IlmImf libpng libwebp libtiff libopenjp2 libjpeg-turbo zlib)
add_test(test_trans test_trans)

