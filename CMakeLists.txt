cmake_minimum_required(VERSION 3.13)

project(fcgi_responder VERSION 1.0.0 DESCRIPTION "FCGI protocole responder role implementation library")


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SRC
    src/decoder.cpp
    src/encoder.cpp
    src/errors.cpp
    src/message.cpp
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
    src/requesteditor.cpp
    src/responder.cpp
    src/response.cpp
    src/streamdatamessage.cpp
    src/streammaker.cpp)

add_library(fcgi_responder STATIC ${SRC})
set_target_properties(fcgi_responder PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(fcgi_responder PROPERTIES PUBLIC_HEADER "include/constants.h;include/request.h;include/response.h;include/responder.h")

target_include_directories(fcgi_responder PRIVATE include)
target_include_directories(fcgi_responder PRIVATE src)

option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS fcgi_responder
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcgi_responder)
