# Architecture overview

C23 required, tested on clang 18

- `src/include` - shared include files
- `src/include/generic` - generic struct definitions (`Vector`, `Hashmap`, ...)
- `src/lib` - shared library source
- `src/cli` - CLI
- `src/gui` - management GUI
- `target` - build artifacts
