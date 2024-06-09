set(SRC Fileoperations.cc)
add_library(local_fileoperations)
target_sources(local_fileoperations PRIVATE ${SRC})
target_include_directories(local_fileoperations PRIVATE ${CMAKE_CURRENT_BINARY_DIR})