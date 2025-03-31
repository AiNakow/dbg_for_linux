cmake_minimum_required(VERSION 3.10.0)
project(dbg_for_linux VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

find_package(libelfin REQUIRED)

set(cmake_runtime_output_directory ${CMAKE_BINARY_DIR}/bin)
set(cmake_library_output_directory ${CMAKE_BINARY_DIR}/lib)
set(cmake_archive_output_directory ${CMAKE_BINARY_DIR}/lib)

foreach(outputconfig ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${outputconfig} outputconfig)
    set(cmake_runtime_output_directory_${outputconfig} ${cmake_runtime_output_directory}/${outputconfig})
    set(cmake_library_output_directory_${outputconfig} ${cmake_library_output_directory}/${outputconfig})
    set(cmake_archive_output_directory_${outputconfig} ${cmake_archive_output_directory}/${outputconfig})
    
endforeach()

add_executable(dbg_for_linux main.cpp ${SOURCES})