cmake_minimum_required(VERSION 3.14)

project(coasyncpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fib
    examples/fib.cpp
)
target_include_directories(fib PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(subcalls
    examples/subcalls.cpp
)
target_include_directories(subcalls PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(multitasking
    examples/multitasking.cpp
)
target_include_directories(multitasking PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(c_callback
    examples/c_callback.cpp
)
target_include_directories(c_callback PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(error_handling
    examples/error_handling.cpp
)
target_include_directories(error_handling PRIVATE "${CMAKE_SOURCE_DIR}/include")