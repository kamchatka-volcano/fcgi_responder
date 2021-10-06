cmake_minimum_required(VERSION 3.18)

project(fcgi_responder VERSION 1.0.0 DESCRIPTION "FCGI protocol responder role implementation library")

if(NOT DEFINED FCGI_RESPONDER_SUBPROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(FCGI_RESPONDER_SUBPROJECT OFF)
    else()
        set(FCGI_RESPONDER_SUBPROJECT ON)
    endif()
endif()

include(CheckPIESupported)
check_pie_supported()

set(SRC
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
    src/responder.cpp
    src/response.cpp
    src/streamdatamessage.cpp
    src/streammaker.cpp
    src/inputstreamdualbuffer.cpp
    src/requestregistry.cpp)

set(PUBLIC_HEADERS
    "include/fcgi_responder/request.h"
    "include/fcgi_responder/response.h"
    "include/fcgi_responder/responder.h"
)

add_library(fcgi_responder STATIC ${SRC})
target_compile_features(fcgi_responder PUBLIC cxx_std_17)
set_target_properties(fcgi_responder PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(fcgi_responder PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
set_target_properties(fcgi_responder PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(fcgi_responder INTERFACE include/)
target_include_directories(fcgi_responder PRIVATE include/)
target_include_directories(fcgi_responder PRIVATE src)

option(ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS AND NOT FCGI_RESPONDER_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()

option(ENABLE_ASIO_EXAMPLE "Build asio example" OFF)
if (ENABLE_ASIO_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-asio)
endif()

option(ENABLE_QT_EXAMPLE "Build Qt example" OFF)
if (ENABLE_QT_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-qt)
endif()

include(GNUInstallDirs)
if(NOT FCGI_RESPONDER_SUBPROJECT)
    install(TARGETS fcgi_responder
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcgi_responder)
endif()