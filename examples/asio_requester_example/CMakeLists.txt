cmake_minimum_required(VERSION 3.18)
project(asio_requester_example)

SealLake_DownloadSource(
        NAME asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
)

SealLake_Executable(
        SOURCES main.cpp
        COMPILE_FEATURES cxx_std_17
        PROPERTIES
            CXX_EXTENSIONS OFF
        INCLUDES
            ${SEAL_LAKE_SOURCE_asio}/asio/include
        LIBRARIES
            fcgi_responder::fcgi_responder asio
)