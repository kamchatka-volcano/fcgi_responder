cmake_minimum_required(VERSION 3.18)
project(fcgi_responder_fuzzer VERSION 1.1.0 DESCRIPTION "FastCGI protocol responder role implementation library")

SealLake_Import(
        cmdlime 1.0.1
        GIT_REPOSITORY "https://github.com/kamchatka-volcano/cmdlime.git"
        GIT_TAG "v1.0.1"
)

SealLake_Executable(
        SOURCES src/main.cpp
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES
            fcgi_responder::fcgi_responder
            cmdlime::cmdlime
)
