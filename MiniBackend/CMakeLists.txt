cmake_minimum_required(VERSION 3.15)

project(MiniBackend)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(
    minibackend

    src/main.cpp

    src/server/Server.cpp

    src/http/Http.cpp

    src/http/Response.cpp

    src/router/Router.cpp
)

target_include_directories(
    minibackend
    PRIVATE
    src
)