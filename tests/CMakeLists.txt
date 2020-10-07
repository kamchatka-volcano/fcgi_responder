cmake_minimum_required(VERSION 2.6)
 
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}, ..)

set(SRC
    test_record_serialization.cpp 
    test_utils.cpp 
    test_responder.cpp)

add_executable(test_fcgi ${SRC})
add_test(NAME test_fcgi COMMAND test_fcgi)
target_include_directories(test_fcgi PRIVATE ../include/fcgi_responder)
target_include_directories(test_fcgi PRIVATE ../src)
target_link_libraries(test_fcgi gtest gmock gtest_main pthread fcgi_responder)
