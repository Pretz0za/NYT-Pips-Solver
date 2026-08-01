cmake_minimum_required(VERSION 3.10)
project(PipsSolver)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(solver
    ${SRC_FILES}
)

target_include_directories(solver PUBLIC ${CMAKE_SOURCE_DIR}/include)
