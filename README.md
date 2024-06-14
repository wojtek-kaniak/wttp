# WTTP
A minimal HTTP 1.1 server written in C23 using BSD sockets.

> [!CAUTION]
> Not production safe.

[Architecture](/ARCHITECTURE.md)

# Features
- basic management GUI
- Content-Type handling

# Building

For dependencies see [shell.nix](/shell.nix).

```sh
# Only once
./setup.sh

# or cmake --build build/
./build.sh

# Artifacts in build/
build/wttp-cli
```
