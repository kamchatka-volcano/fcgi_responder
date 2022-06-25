cmake_minimum_required(VERSION 3.18)
project(libfcgi_benchmark)

include(external/cmdlime)

set(SRC
        libfcgi_benchmark.cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PRIVATE fcgi fcgi++ cmdlime::cmdlime)
