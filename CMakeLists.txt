cmake_minimum_required(VERSION 3.0)

project(lend CXX)
set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

## Set CXX optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wpedantic -Wall -Wextra -Werror")
# -mtune=native -march=native

## Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
else(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
endif(CMAKE_BUILD_TYPE STREQUAL "Release")
message(STATUS "Flags: " ${CMAKE_CXX_FLAGS})

## Dependencies
find_package(zydis REQUIRED)

include_directories(include ${zydis_INCLUDE_DIR})
link_directories(${zydis_LIB_DIR})

add_executable(ld64_tables src/ld64_tables.cpp)

add_executable(ld64_test src/ld64_test.cpp)
target_link_libraries(ld64_test "Zydis")

add_executable(ld64asm_test src/ld64asm_test.cpp)
target_link_libraries(ld64asm_test "Zydis")
