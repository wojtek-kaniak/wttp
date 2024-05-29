cmake_minimum_required(VERSION 3.10)

project(WTTP)

add_executable(WTTP src/main.c)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

add_subdirectory(src/lib)
