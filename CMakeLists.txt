cmake_minimum_required(VERSION 3.10.0)
project(edb VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
set(LIBDWARF ${CMAKE_SOURCE_DIR}/lib/libelfin/dwarf/libdwarf++.a)
set(LIBELF ${CMAKE_SOURCE_DIR}/lib/libelfin/elf/libelf++.a)
set(LIBLINENOISE ${CMAKE_SOURCE_DIR}/lib/linenoise/liblinenoise.a)
file(GLOB SOURCES "src/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(outputconfig ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} outputconfig)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${OUTPUTCONFIG})
    
endforeach()

add_executable(edb main.cpp ${SOURCES})
target_link_libraries(edb ${LIBDWARF} ${LIBELF} ${LIBLINENOISE})

target_include_directories(edb PRIVATE ${CMAKE_SOURCE_DIR}/lib/libelfin)
target_include_directories(edb PRIVATE ${CMAKE_SOURCE_DIR}/lib/linenoise)