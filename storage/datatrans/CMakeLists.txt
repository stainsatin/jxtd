set(SRC DatatranServer.cc)
add_library(storage_datatrans)
target_sources(storage_datatrans PRIVATE ${SRC})
target_include_directories(storage_datatrans PRIVATE ${CMAKE_BINARY_DIR} {CMAKE_BINARY_DIR}/storage/datamanagement)
add_dependencies(storage_datatrans workflow-0.10.4 datatransfer_proto)

add_executable(test_DatatranServer)
target_include_directories(test_DatatranServer PRIVATE ${CMAKE_BINARY_DIR} {CMAKE_BINARY_DIR}/storage/datamanagement)
target_sources(test_DatatranServer PRIVATE test_DatatranServer.cc)
target_link_libraries(test_DatatranServer PRIVATE storage_datatrans datatransfer_msg datatransfer_proto datamanagement 
    taskdep_message taskdep_proto local_fileoperations datatransfer_client fs_scheduler workflow gtest gtest_main protobuf pthread)
add_dependencies(test_DatatranServer workflow-0.10.4 gtest-1.12.0)
add_test(test_DatatranServer test_DatatranServer)
