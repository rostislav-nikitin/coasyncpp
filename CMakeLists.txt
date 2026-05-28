cmake_minimum_required(VERSION 3.14)

project(coasyncpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

add_executable(core_call
    examples/core_call.cpp
)
target_include_directories(core_call PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(expected_call
    examples/expected_call.cpp
)
target_include_directories(expected_call PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(variant_call
    examples/variant_call.cpp
)
target_include_directories(variant_call PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(fib
    examples/fib.cpp
)
target_include_directories(fib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(subcalls
    examples/subcalls.cpp
)
target_include_directories(subcalls PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(multitasking
    examples/multitasking.cpp
)
target_include_directories(multitasking PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(c_callback
    examples/c_callback.cpp
)
target_include_directories(c_callback PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(c_callback_with_error
    examples/c_callback_with_error.cpp
)
target_include_directories(c_callback_with_error PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(error_handling
    examples/error_handling.cpp
)
target_include_directories(error_handling PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(vector
    examples/vector.cpp
)
target_include_directories(vector PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
