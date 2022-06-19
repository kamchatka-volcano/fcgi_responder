cmake_minimum_required(VERSION 3.18)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SRC
        input_generating_test_responder.cpp)

add_executable(fuzz_input_generator ${SRC})
target_compile_features(fuzz_input_generator PUBLIC cxx_std_17)
set_target_properties(fuzz_input_generator PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(fuzz_input_generator PRIVATE ../../src)
target_link_libraries(fuzz_input_generator PRIVATE ${GTEST_BOTH_LIBRARIES} gmock Threads::Threads fcgi_responder::fcgi_responder)
