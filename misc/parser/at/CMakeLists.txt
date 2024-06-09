include(FindBISON)
include(FindFLEX)
find_package(BISON 2.3)
find_package(FLEX 2.3)
if(NOT FLEX_FOUND)
    message(FATAL_ERROR "flex>=2.3 not found")
endif()
if(NOT BISON_FOUND)
    message(FATAL_ERROR "bison>=2.3 not found")
endif()

bison_target(configparser
             config.y
             ${CMAKE_CURRENT_BINARY_DIR}/parser.c
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)
flex_target(configlexer
            token.l
            ${CMAKE_CURRENT_BINARY_DIR}/token.c )
add_flex_bison_dependency(configlexer configparser)

set(SRC action.c)
add_library(ATAction)
target_sources(ATAction PRIVATE ${SRC})
target_include_directories(ATAction PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(SRC ATParser.cc)
add_library(ATParser)
target_sources(ATParser PRIVATE ${SRC} ${BISON_configparser_OUTPUTS} ${FLEX_configlexer_OUTPUTS})
target_include_directories(ATParser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(SRC test_ATParser.cc)
add_executable(test_ATParser)
target_sources(test_ATParser PRIVATE ${SRC} ${BISON_configparser_OUTPUTS} ${FLEX_configlexer_OUTPUTS})
target_include_directories(test_ATParser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_ATParser PRIVATE ATParser gtest_main gtest pthread ATAction)
add_dependencies(test_ATParser gtest-1.12.0)
add_test(test_Parser_ATParser test_ATParser)
