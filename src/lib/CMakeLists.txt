file(GLOB LIB_SOURCES *.c)
file(GLOB LIB_HEADERS *.h)

add_library(libwttp ${LIB_SOURCES} ${LIB_HEADERS})

set(C_STANDARD 23)
# set(CMAKE_C_STANDARD_REQUIRED True)

target_include_directories(libwttp PRIVATE "../include")

target_compile_definitions(libwttp PRIVATE WTTP_P_LIB_IMPL)
target_compile_definitions(libwttp PRIVATE _XOPEN_SOURCE=500 POSIX_C_SOURCE=200112L)

target_compile_definitions(libwttp PRIVATE DEBUG=1)

# target_compile_definitions(libwttp PRIVATE
# 	$<$<CONFIG:Debug>:
# 		DEBUG=1
# 	>
# )
