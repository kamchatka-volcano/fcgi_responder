cmake_minimum_required(VERSION 3.18)
project(qt_example)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

SealLake_Executable(
        SOURCES main.cpp
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES
            fcgi_responder::fcgi_responder
            Qt5::Core
            Qt5::Network
)