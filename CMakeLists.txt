cmake_minimum_required(VERSION 3.20)

project(solar-calculations)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)

find_package(Rust REQUIRED)

add_library(sun cpp/wiki_sun.cpp cpp/noaa_sun.cpp)

add_library(redshift_solar cpp/redshift_solar.c cpp/redshift_solar.cpp)

add_rust_library(TARGET solar_calc SOURCE_DIRECTORY ${CMAKE_SOURCE_DIR}/rust BINARY_DIRECTORY ${CMAKE_BINARY_DIR})

link_libraries(date-tz fmt)

add_executable(sunrise-test cpp/sunrise-test.cpp)
target_link_libraries(sunrise-test PRIVATE sun redshift_solar solar_calc)

add_executable(bench cpp/bench.cpp)
target_link_libraries(bench PRIVATE sun redshift_solar solar_calc benchmark)
