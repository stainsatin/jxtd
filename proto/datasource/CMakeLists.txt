protolib(datasource_proto ProtocolMessage)

set(SRC response.cc request.cc)
add_library(datasource_msg)
target_sources(datasource_msg PRIVATE ${SRC})
target_include_directories(datasource_msg PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(datasource_msg datasource_proto workflow-0.10.4 protobuf-3.21.12)
