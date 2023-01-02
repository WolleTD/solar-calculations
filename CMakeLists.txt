cmake_minimum_required(VERSION 3.20)

project(solar-calculations)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)

find_package(Rust REQUIRED)

add_library(sun cpp/sun.cpp cpp/sun2.cpp)
target_link_libraries(sun PUBLIC date-tz fmt)

add_library(solar cpp/solar.c cpp/solar.cpp)

add_rust_library(TARGET solar_calc SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} BINARY_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(sunrise-test cpp/sunrise-test.cpp)
target_link_libraries(sunrise-test PRIVATE sun solar solar_calc)

add_executable(bench cpp/bench.cpp)
target_link_libraries(bench PRIVATE sun solar solar_calc benchmark)
