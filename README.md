# Build

Requires:

- Visual Studios 2022
- LLVM-MSVC v143
- CMake

```
cmake -B .build -T LLVM-MSVC_v143 -A x64
cd .build
cmake --build . --config Release
```