cmake_minimum_required(VERSION 3.10.0)
project(example VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB SOURCE "./*.cpp")
add_executable(example ${SOURCE})