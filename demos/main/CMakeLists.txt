cmake_minimum_required(VERSION 3.23)
project(playgroundEngineMainDemo)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3")
endif()

add_executable(playgroundEngineMainDemo
    src/main.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(playgroundEngineMainDemo PUBLIC playgroundEngineLib)
target_include_directories(playgroundEngineMainDemo PUBLIC "../../src")

