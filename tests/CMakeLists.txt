cmake_minimum_required(VERSION 3.18)
project(test_fcgi)

enable_testing()
include(external/googletest)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SRC
    test_record_serialization.cpp
    test_request.cpp
    test_utils.cpp
    test_responder.cpp
    test_requester.cpp)

add_executable(test_fcgi ${SRC})
target_compile_features(test_fcgi PUBLIC cxx_std_17)
set_target_properties(test_fcgi PROPERTIES CXX_EXTENSIONS OFF)

add_test(NAME test_fcgi COMMAND test_fcgi)
target_include_directories(test_fcgi PRIVATE ../include/fcgi_responder)
target_include_directories(test_fcgi PRIVATE ../src)
target_link_libraries(test_fcgi PRIVATE GTest::gtest_main GTest::gmock_main Threads::Threads fcgi_responder::fcgi_responder)

include(GoogleTest)
gtest_discover_tests(test_fcgi)
