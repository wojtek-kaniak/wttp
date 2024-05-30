cmake_minimum_required(VERSION 3.10)

project(WTTP)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall)

file(GLOB CLI_SOURCES src/cli/*.c)
file(GLOB CLI_HEADERS src/cli/*.h)
add_executable(wttp-cli ${CLI_SOURCES} ${CLI_HEADERS})

target_include_directories(wttp-cli PRIVATE src/include)

add_subdirectory(src/lib)

target_link_libraries(wttp-cli libwttp)
