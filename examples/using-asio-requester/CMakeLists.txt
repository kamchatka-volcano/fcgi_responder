cmake_minimum_required(VERSION 3.18)
project(asio_requester_example)

include(external/asio)

set(SRC main.cpp)

add_executable(asio_requester_example ${SRC})
target_compile_features(asio_requester_example PUBLIC cxx_std_17)
set_target_properties(asio_requester_example PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(asio_requester_example PRIVATE fcgi_responder::fcgi_responder asio)
