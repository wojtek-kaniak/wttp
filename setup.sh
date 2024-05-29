#! /bin/sh
# TODO: build system
src_include_path=$(realpath ./src/include)

cat << EOF > .clangd
CompileFlags:
	Add:
		- "-std=c23"
		- "-I$src_include_path"
		- "-Wall"
		- "-Wextra"
		- "-xc"
		- "-DDEBUG=1"
		- "-DWTTP_P_LIB_IMPL"
		- "-D_XOPEN_SOURCE=500"
		- "-DPOSIX_C_SOURCE=200112L"
EOF
