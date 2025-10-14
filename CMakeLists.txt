cmake_minimum_required(VERSION 3.10)
project(PipsSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(solver src/main.cpp)

target_include_directories(solver PUBLIC ${CMAKE_SOURCE_DIR}/include)
