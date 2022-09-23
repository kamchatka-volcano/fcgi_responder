cmake_minimum_required(VERSION 3.18)
project(asio_example)
include(external/asio)

SealLake_Executable(
        SOURCES main.cpp
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES
            fcgi_responder::fcgi_responder asio
)
