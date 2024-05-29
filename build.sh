#! /bin/sh
# TODO: replace with an actual build system
mkdir -p target
cc -Wall -Wextra -std=c23 -fuse-ld=lld -fsanitize=undefined -O1 -Isrc/include src/main.c -o target/main "$@"
