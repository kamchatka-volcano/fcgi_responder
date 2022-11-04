cmake_minimum_required(VERSION 3.18)
project(fcgi_responder_benchmark)
include(external/asio)

SealLake_Import(
        cmdlime 1.0.1
        GIT_REPOSITORY "https://github.com/kamchatka-volcano/cmdlime.git"
        GIT_TAG "v1.0.1"
)

SealLake_Executable(
        SOURCES fcgi_responder_benchmark.cpp
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES
            asio
            fcgi_responder::fcgi_responder
            cmdlime::cmdlime
)
