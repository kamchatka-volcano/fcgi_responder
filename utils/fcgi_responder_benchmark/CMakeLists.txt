cmake_minimum_required(VERSION 3.18)
project(fcgi_responder_benchmark)

include(external/asio)
include(external/cmdlime)

set(SRC
        fcgi_responder_benchmark.cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PRIVATE asio fcgi_responder::fcgi_responder cmdlime::cmdlime)
