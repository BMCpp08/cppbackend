cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

get_property(importTargets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message(STATUS "${importTargets}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(game_lib STATIC
    src/model.h
    src/model.cpp
    src/loot_generator.h
    src/loot_generator.cpp
    src/tagged.h
    src/extra_data.h
    src/model_datails.h
)

target_include_directories(game_lib PUBLIC
        ${CMAKE_SOURCE_DIR}src/
        ${CONAN_INCLUDE_DIRS}
)

# Добавляем библиотеку collision_detection_lib
add_library(collision_detection_lib STATIC
	src/collision_detector.h
	src/collision_detector.cpp
	src/geom.h
)

target_link_libraries(collision_detection_lib PUBLIC 
			CONAN_PKG::boost 
			Threads::Threads)

target_link_libraries(game_lib PUBLIC
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
	tests/collision-detector-tests.cpp
	tests/state-serialization-tests.cpp
)

target_link_libraries(game_server PRIVATE game_lib collision_detection_lib Threads::Threads)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 game_lib collision_detection_lib)
