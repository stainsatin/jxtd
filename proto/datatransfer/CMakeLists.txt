protolib(datatransfer_proto ProtocolMessage)

set(SRC message.cc)
add_library(datatransfer_msg)
target_sources(datatransfer_msg PRIVATE ${SRC})
target_include_directories(datatransfer_msg PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(datatransfer_msg datatransfer_proto workflow-0.10.4 protobuf-3.21.12)
