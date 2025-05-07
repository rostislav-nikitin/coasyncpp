cmake_minimum_required(VERSION 3.14)

project(coasyncpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fib
    examples/fib.cpp
)

target_include_directories(fib PRIVATE "${CMAKE_SOURCE_DIR}/include")