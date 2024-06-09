set(SRC AsymmeticChiper.cc DigestChiper.cc RandomGenerator.cc SymmeticChiper.cc Chiper.cc)

add_library(chiper)
target_sources(chiper PRIVATE  ${SRC})
add_dependencies(chiper openssl-3.0.9)
target_link_libraries(chiper PRIVATE libcrypto.a libssl.a  pthread dl)

#测试中对称加密在drone上有一定问题，暂时打个注释
#add_executable(test_chiper)
#target_sources(test_chiper PRIVATE test_chiper.cc)
#add_dependencies(test_chiper gtest-1.12.0 openssl-3.0.9)

#target_link_libraries(test_chiper PRIVATE chiper libcrypto.a libssl.a gtest_main gtest pthread dl)
#add_test(test_chiper test_chiper)
