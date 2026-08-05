cmake_minimum_required(VERSION 3.15)

project(MiniBackend)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json CONFIG REQUIRED)

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

    src/controller/UserController.cpp

    src/service/UserService.cpp
)

target_include_directories(
    minibackend
    PRIVATE
    src
)

target_link_libraries(
    minibackend
    PRIVATE
    nlohmann_json::nlohmann_json
)