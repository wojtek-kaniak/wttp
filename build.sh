#! /bin/sh

if ! grep 'project(WTTP)' CMakeLists.txt >/dev/null ; then
	printf 'setup.sh should be called in the project root directory'
	exit 2
fi

cmake --build build/ --config Debug
