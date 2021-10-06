cmake_minimum_required(VERSION 3.18)
project(qt_example)

set(SRC main.cpp)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

add_executable(qt_example ${SRC})
target_compile_features(qt_example PUBLIC cxx_std_17)
set_target_properties(qt_example PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(qt_example PRIVATE fcgi_responder Qt5::Core Qt5::Network)
