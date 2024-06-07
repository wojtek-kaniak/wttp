#! /bin/sh

cd "$(dirname $0)"

# TODO: x86
cd nativefiledialog/build/gmake_linux/
make config=release_x64 all
cd ../../..

cp nativefiledialog/build/lib/Release/x64/libnfd.a .
