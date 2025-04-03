cmake_minimum_required(VERSION 3.10)
project(linenoise VERSION 1.0 LANGUAGES C)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_library(linenoise STATIC linenoise.c)

target_include_directories(linenoise PUBLIC ${CMAKE_SOURCE_DIR})