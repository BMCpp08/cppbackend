cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

get_property(importTargets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message(STATUS "${importTargets}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Добавляем библиотеку, указывая, что она статическая.
add_library(GameLib STATIC
    src/model.h
    src/model.cpp
    src/loot_generator.h
    src/loot_generator.cpp
    src/tagged.h
    src/extra_data.h
    src/extra_data.cpp
)

target_include_directories(GameLib PUBLIC
        ${CMAKE_SOURCE_DIR}src/
        ${CONAN_INCLUDE_DIRS}
)

target_link_libraries(GameLib PUBLIC
        CONAN_PKG::boost
        Threads::Threads
)

add_executable(game_server
    src/boost_includes.h
    src/main.cpp
    src/http_server.cpp
    src/http_server.h
    src/sdk.h
    src/boost_json.cpp
    src/json_loader.h
    src/json_loader.cpp
    src/request_handler.cpp
    src/request_handler.h
    src/logger.h
    src/app.h
    src/app.cpp
    src/ticker.h
    src/ticker.cpp
)

add_executable(game_server_tests
    tests/loot_generator_tests.cpp
	tests/model_testes.cpp
)


#target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)

target_link_libraries(game_server PRIVATE GameLib Threads::Threads)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 GameLib)
