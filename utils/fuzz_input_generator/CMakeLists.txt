cmake_minimum_required(VERSION 3.18)
project(fuzz_input_generator)

SealLake_GoogleTest(
        SOURCES input_generating_test_responder.cpp
        INCLUDES ../../src
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        LIBRARIES fcgi_responder::fcgi_responder
)