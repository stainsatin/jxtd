set(SRC main.cc)
add_executable(management_main)
target_sources(management_main PRIVATE ${SRC})
target_include_directories(management_main PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/proto/datasource)
target_link_libraries(management_main PRIVATE taskdep_management taskdep_message taskdep_proto  management_datasource datasource_msg datasource_proto workflow protobuf pthread flags gflags)
add_dependencies(management_main protobuf-3.21.12 workflow-0.10.4 gflags-2.2.2)

