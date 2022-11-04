cmake_minimum_required(VERSION 3.18)
project(test_fcgi_responder)

SealLake_GoogleTest(
    SOURCES
        test_record_serialization.cpp
        test_request.cpp
        test_utils.cpp
        test_responder.cpp
        test_requester.cpp
        test_datareaderstream.cpp
    INCLUDES
        ../src
    LIBRARIES
        fcgi_responder::fcgi_responder
)