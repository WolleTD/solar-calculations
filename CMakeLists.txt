cmake_minimum_required(VERSION 3.20)

project(solar-calculations)

set(CMAKE_CXX_STANDARD 17)

add_library(sun cpp/sun.cpp)
target_link_libraries(sun PUBLIC date-tz fmt)

add_library(solar cpp/solar.c cpp/solar.cpp)

add_executable(sunrise-test cpp/sunrise-test.cpp)
target_link_libraries(sunrise-test PRIVATE sun solar)

add_executable(bench cpp/bench.cpp)
target_link_libraries(bench PRIVATE sun solar benchmark)
