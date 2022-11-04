cmake_minimum_required(VERSION 3.18)
project(fcgi_responder VERSION 1.4.1)

include(GNUInstallDirs)
include(external/seal_lake)

set(SRC
    src/datareaderstream.cpp
    src/datawriterstream.cpp
    src/decoder.cpp
    src/encoder.cpp
    src/errors.cpp
    src/msgabortrequest.cpp
    src/msgbeginrequest.cpp
    src/msgendrequest.cpp
    src/msggetvalues.cpp
    src/msggetvaluesresult.cpp
    src/msgparams.cpp
    src/msgunknowntype.cpp
    src/namevalue.cpp
    src/record.cpp
    src/recordreader.cpp
    src/request.cpp
    src/requestdata.cpp
    src/responder.cpp
    src/responderimpl.cpp
    src/requester.cpp
    src/requesterimpl.cpp
    src/response.cpp
)

set(PUBLIC_HEADERS
    "include/fcgi_responder/request.h"
    "include/fcgi_responder/response.h"
    "include/fcgi_responder/responder.h"
    "include/fcgi_responder/requester.h"
)

SealLake_StaticLibrary(
        SOURCES
            ${SRC}
        COMPILE_FEATURES cxx_std_17
        PUBLIC_HEADERS ${PUBLIC_HEADERS}
        PROPERTIES
            CXX_EXTENSIONS OFF
            POSITION_INDEPENDENT_CODE ON
)

SealLake_OptionalBuildSteps(
        tests
        fuzz_tests
        utils/fuzz_input_generator
        examples/asio_example
        examples/asio_requester_example
        examples/qt_example
        examples/qt_requester_example
        utils/libfcgi_benchmark
        utils/fcgi_responder_benchmark
)
