cmake_minimum_required(VERSION 3.18)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SRC
    test_record_serialization.cpp 
    test_request.cpp 
    test_utils.cpp 
    test_responder.cpp)

add_executable(test_fcgi ${SRC})
target_compile_features(test_fcgi PUBLIC cxx_std_17)
set_target_properties(test_fcgi PROPERTIES CXX_EXTENSIONS OFF)

add_test(NAME test_fcgi COMMAND test_fcgi)
target_include_directories(test_fcgi PRIVATE ../include/fcgi_responder)
target_include_directories(test_fcgi PRIVATE ../src)
target_link_libraries(test_fcgi PRIVATE gtest gmock gtest_main Threads::Threads fcgi_responder)
