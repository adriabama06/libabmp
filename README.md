# libabmp

A lightweight C library for reading and writing BMP files, implemented from scratch without external dependencies.

## Features
- Supports 24-bit and 32-bit BMP formats
- Pure C implementation (no external libraries)
- Minimal overhead with direct file I/O

## Build Instructions

### All Platforms
```bash
cmake -B build
cmake --build build
```

> **Note**: On Windows, ensure MinGW is installed and available in PATH. The library does not compile with MSVC (cl.exe) due to C language feature requirements.

### Example Usage
```bash
cd example
cmake -B build
cmake --build build
./build/example
```
The example generates `output.bmp` using sample images from `samples/`.

## Project Structure
- `include/`: Public headers (`abitmap.h`, `abmp.h`)
- `src/`: Library implementation (24+ source files)
- `example/`: Demonstrates library usage
- `samples/`: Input/output sample images

## License
Distributed under the MIT License. See [LICENSE](LICENSE) for details.
