cmake_minimum_required(VERSION 3.15)

project(MiniBackend)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json CONFIG REQUIRED)

find_package(SQLite3 REQUIRED)

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

    src/repository/UserRepository.cpp

    src/middleware/LoggerMiddleware.h

    src/middleware/TimerMiddleware.h
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
    SQLite3::SQLite3
)

include(CTest)

if(BUILD_TESTING)
    add_executable(
        user_repository_test
        tests/UserRepositoryTest.cpp
        src/repository/UserRepository.cpp
    )

    target_include_directories(
        user_repository_test
        PRIVATE
        src
    )

    target_link_libraries(
        user_repository_test
        PRIVATE
        SQLite3::SQLite3
    )

    add_test(
        NAME user_repository_persistence
        COMMAND user_repository_test
    )

    add_executable(
        http_parser_test
        tests/HttpParserTest.cpp
        src/http/Http.cpp
    )

    target_include_directories(
        http_parser_test
        PRIVATE
        src
    )

    add_test(
        NAME http_parser
        COMMAND http_parser_test
    )
endif()
