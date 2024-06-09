macro(protolib libname protoname)
    add_custom_target(${libname}_target
        ${PROJECT_SOURCE_DIR}/thirdparty/protobuf-3.21.12/bin/protoc -I ${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${protoname}.proto
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${protoname}.pb.h ${CMAKE_CURRENT_BINARY_DIR}/${protoname}.pb.cc
        SOURCES ${protoname}.proto
    )
    add_dependencies(${libname}_target protobuf-3.21.12)
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/${protoname}.pb.cc
        PROPERTIES
            GENERATED TRUE
    )
    add_library(${libname})
    target_sources(${libname} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${protoname}.pb.cc)
endmacro()
