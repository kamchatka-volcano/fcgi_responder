cmake_minimum_required(VERSION 3.18)

project(fcgi_responder VERSION 1.2.0 DESCRIPTION "FastCGI protocol responder role implementation library")

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(FCGI_RESPONDER_SUBPROJECT OFF)
else()
    set(FCGI_RESPONDER_SUBPROJECT ON)
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
    src/responderimpl.cpp
    src/requester.cpp
    src/requesterimpl.cpp
    src/response.cpp
    src/inputstreamdualbuffer.cpp
    src/requestregistry.cpp)

set(PUBLIC_HEADERS
    "include/fcgi_responder/request.h"
    "include/fcgi_responder/response.h"
    "include/fcgi_responder/responder.h"
    "include/fcgi_responder/requester.h"
    "include/fcgi_responder/requesthandle.h"
)

add_library(fcgi_responder STATIC ${SRC})
add_library(fcgi_responder::fcgi_responder ALIAS fcgi_responder)
target_compile_features(fcgi_responder PUBLIC cxx_std_17)
set_target_properties(fcgi_responder PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(fcgi_responder PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
set_target_properties(fcgi_responder PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(fcgi_responder PRIVATE include/)
target_include_directories(fcgi_responder PRIVATE src)
target_include_directories(
        fcgi_responder
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


option(ENABLE_TESTS "Enable tests" OFF)
if (ENABLE_TESTS AND NOT FCGI_RESPONDER_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()

option(ENABLE_FUZZ_TESTS "Enable fuzz tests" ON)
if (ENABLE_FUZZ_TESTS AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(fuzz_tests)
endif()

option(ENABLE_FUZZ_INPUT_GENERATOR "Enable fuzz input generator" ON)
if (ENABLE_FUZZ_INPUT_GENERATOR AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(utils/fuzz_input_generator)
endif()

option(ENABLE_ASIO_EXAMPLE "Build asio example" OFF)
if (ENABLE_ASIO_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-asio)
endif()

option(ENABLE_ASIO_REQUESTER_EXAMPLE "Build asio requester example" OFF)
if (ENABLE_ASIO_REQUESTER_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-asio-requester)
endif()

option(ENABLE_QT_EXAMPLE "Build Qt example" OFF)
if (ENABLE_QT_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-qt)
endif()

option(ENABLE_QT_REQUESTER_EXAMPLE "Build qt requester example" OFF)
if (ENABLE_QT_REQUESTER_EXAMPLE AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(examples/using-qt-requester)
endif()

option(ENABLE_LIBFCGI_BENCHMARK "Enable fuzz input generator" ON)
if (ENABLE_LIBFCGI_BENCHMARK AND NOT FCGI_RESPONDER_SUBPROJECT)
    add_subdirectory(utils/libfcgi_benchmark)
endif()

option(INSTALL_FCGI_RESPONDER "Install fcgi_responder library unconditionally" OFF)
if(NOT FCGI_RESPONDER_SUBPROJECT OR INSTALL_FCGI_RESPONDER)
    include(GNUInstallDirs)

    set(FCGI_RESPONDER_PACK_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/fcgi_responder)
    install(TARGETS fcgi_responder
            EXPORT fcgi_responder-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcgi_responder
    )
    install(EXPORT fcgi_responder-targets
            FILE fcgi_responderTargets.cmake
            NAMESPACE fcgi_responder::
            DESTINATION ${FCGI_RESPONDER_PACK_PATH}
    )

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/fcgi_responderConfigVersion.cmake
            COMPATIBILITY SameMajorVersion
    )
    configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/fcgi_responderConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/fcgi_responderConfig.cmake
            INSTALL_DESTINATION ${FCGI_RESPONDER_PACK_PATH}
    )
    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/fcgi_responderConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/fcgi_responderConfigVersion.cmake
            DESTINATION ${FCGI_RESPONDER_PACK_PATH}
    )
endif()