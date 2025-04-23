cmake_minimum_required(VERSION 3.10.0)
project(example VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -gdwarf-4")

# 设置默认构建类型为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB SOURCE "./*.cpp")
add_executable(example ${SOURCE})