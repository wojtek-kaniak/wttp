cmake_minimum_required(VERSION 3.10)

project(WTTP)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall)

# CLI:
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_options(-O0 -g3 -fsanitize=undefined)
	add_link_options(-g3 -fsanitize=undefined)
endif()

file(GLOB CLI_SOURCES src/cli/*.c)
file(GLOB CLI_HEADERS src/cli/*.h)
add_executable(wttp-cli ${CLI_SOURCES} ${CLI_HEADERS})

target_include_directories(wttp-cli PRIVATE src/include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(wttp-cli PRIVATE DEBUG=1)
endif()

# GUI:
file(GLOB GUI_SOURCES src/gui/*.c)
file(GLOB GUI_HEADERS src/gui/*.h)

add_executable(wttp-gui ${GUI_SOURCES} ${GUI_HEADERS}
	src/gui/deps/libnfd.a ${CMAKE_CURRENT_BINARY_DIR}/assets/gschemas.compiled)

target_include_directories(wttp-gui PRIVATE src/include)
target_include_directories(wttp-gui PRIVATE src/gui/deps/nativefiledialog/src/include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(wttp-gui PRIVATE DEBUG=1)
endif()

target_link_libraries(wttp-gui -lraylib -lgtk-3 -lglib-2.0)

# NativeFileDialog
add_custom_command(
	OUTPUT src/gui/deps/libnfd.a
	COMMAND ${CMAKE_SOURCE_DIR}/src/gui/deps/build.sh
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets/)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets/gschemas.compiled
	COMMAND glib-compile-schemas ${CMAKE_SOURCE_DIR}/src/gui/deps/data --targetdir=${CMAKE_CURRENT_BINARY_DIR}/assets/
)

target_link_libraries(wttp-gui ${CMAKE_SOURCE_DIR}/src/gui/deps/libnfd.a)

# libwttp
add_subdirectory(src/lib)

target_link_libraries(wttp-cli libwttp)
target_link_libraries(wttp-gui libwttp)
