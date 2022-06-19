cmake_minimum_required(VERSION 3.18)

project(fcgi_responder_fuzzer VERSION 1.1.0 DESCRIPTION "FastCGI protocol responder role implementation library")

set(SRC
    src/main.cpp)

include(FetchContent)
FetchContent_Declare(cmdlime
    GIT_REPOSITORY "https://github.com/kamchatka-volcano/cmdlime.git"
    GIT_TAG "v1.0.1"
)
FetchContent_MakeAvailable(cmdlime)

add_executable(${PROJECT_NAME} ${SRC})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(${PROJECT_NAME} PRIVATE fcgi_responder::fcgi_responder cmdlime::cmdlime )