file(GLOB LIB_SOURCES *.c **/*.c)
file(GLOB LIB_HEADERS *.h **/*.h)

add_library(libwttp ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(libwttp PRIVATE "../include")

target_compile_definitions(libwttp PRIVATE WTTP_P_LIB_IMPL)
target_compile_definitions(libwttp PRIVATE _XOPEN_SOURCE=500 POSIX_C_SOURCE=200112L)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(libwttp PRIVATE DEBUG=1)
endif()

# target_compile_definitions(libwttp PRIVATE
# 	$<$<CONFIG:Debug>:
# 		DEBUG=1
# 	>
# )
