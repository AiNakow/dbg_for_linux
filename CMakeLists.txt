cmake_minimum_required(VERSION 3.10.0)
project(dbg_for_linux VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
set(LIBDWARF, ${CMAKE_SOURCE_DIR}/lib/libelfin/dwarf/libdwarf++.a)
set(LIBELF, ${CMAKE_SOURCE_DIR}/lib/libelfin/elf/libelf++.a)
set(LIBLINENOISE ${CMAKE_SOURCE_DIR}/lib/linenoise/liblinenoise.a)
file(GLOB SOURCES "src/*.cpp")

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
target_link_libraries(dbg_for_linux ${LIBDWARF} ${LIBELF} ${LIBLINENOISE})

target_include_directories(dbg_for_linux PRIVATE ${CMAKE_SOURCE_DIR}/lib/libelfin)
target_include_directories(dbg_for_linux PRIVATE ${CMAKE_SOURCE_DIR}/lib/linenoise)