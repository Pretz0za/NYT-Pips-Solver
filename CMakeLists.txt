cmake_minimum_required(VERSION 3.10)
project(PipsSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(solver
    ${SRC_FILES}
)

target_include_directories(solver PUBLIC ${CMAKE_SOURCE_DIR}/include)
