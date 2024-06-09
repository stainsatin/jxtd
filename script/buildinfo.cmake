set(BUILDINFO_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
function(write_buildinfo)
    string(TIMESTAMP COMPILE_TIME %Y%m%d%H%M%S%f%Z)
    execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    file(GLOB THIRDPARTY_DEPENDENCIES
        LIST_DIRECTORIES TRUE
        RELATIVE ${CMAKE_SOURCE_DIR}/vendors
        ${CMAKE_SOURCE_DIR}/vendors/*
    )
    configure_file(
        ${BUILDINFO_MODULE_PATH}/config.h.in
        ${CMAKE_BINARY_DIR}/config.h
    )
endfunction()
