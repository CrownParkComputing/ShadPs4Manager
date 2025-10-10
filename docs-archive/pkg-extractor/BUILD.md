# Building PKG Extractor CLI

Build instructions for the command-line PKG extractor.

## Quick Build

```bash
# From repository root
./build.sh

# Binary location
ls -lh build/bin/shadps4-pkg-extractor
```

## Prerequisites

### All Platforms
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Crypto++ 8.9.0 (included as submodule)
- zlib (included as submodule)

**No Qt required** - This is a CLI-only tool.

### Linux

```bash
sudo apt install build-essential cmake git
```

### macOS

```bash
xcode-select --install
brew install cmake
```

### Windows

Install Visual Studio 2022 with C++ support.

## Build Steps

### Linux / macOS

```bash
# Clone repository
git clone https://github.com/CrownParkComputing/ShadPs4Manager.git
cd ShadPs4Manager

# Initialize submodules
git submodule update --init --recursive

# Build
mkdir -p build && cd build
cmake ..
make shadps4-pkg-extractor -j$(nproc)

# Test
./bin/shadps4-pkg-extractor --help
```

### Windows

```bash
# Open "x64 Native Tools Command Prompt for VS 2022"
cd C:\path\to\ShadPs4Manager

mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --target shadps4-pkg-extractor

# Test
bin\Release\shadps4-pkg-extractor.exe --help
```

## CMake Options

Build only the CLI tool:

```bash
cmake .. -DBUILD_GUI=OFF -DBUILD_UNLOCK_GENERATOR=OFF
make shadps4-pkg-extractor
```

Static build:

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF
make shadps4-pkg-extractor
```

Optimized build:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
make shadps4-pkg-extractor
```

## Dependencies

The CLI tool depends on:
- **shadps4_manager_core** library
- **Crypto++** (for PKG decryption)
- **zlib** (for compression)

All are built automatically from submodules.

## Testing

```bash
cd build/bin

# Test with actual PKG file
./shadps4-pkg-extractor /path/to/game.pkg /tmp/output --verbose

# Check binary size
ls -lh shadps4-pkg-extractor

# Check dependencies (Linux)
ldd shadps4-pkg-extractor
```

Expected output:
```
shadps4-pkg-extractor: ~3.0 MB
Dependencies: libc, libstdc++, libm (system libraries only)
```

## Installation

```bash
# System-wide (Linux/macOS)
sudo cp build/bin/shadps4-pkg-extractor /usr/local/bin/
sudo chmod +x /usr/local/bin/shadps4-pkg-extractor

# Verify
shadps4-pkg-extractor --version
```

## Cross-Compilation

### Linux → Windows (MinGW)

```bash
sudo apt install mingw-w64
mkdir build-windows && cd build-windows

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
  -DBUILD_GUI=OFF \
  -DBUILD_UNLOCK_GENERATOR=OFF

make shadps4-pkg-extractor
```

## Troubleshooting

### Crypto++ Not Found

```bash
# Ensure submodules initialized
git submodule update --init --recursive
```

### Linking Errors

```bash
# Clean rebuild
rm -rf build
mkdir build && cd build
cmake ..
make shadps4-pkg-extractor -j$(nproc)
```

### Binary Too Large

```bash
# Strip debug symbols
strip build/bin/shadps4-pkg-extractor

# Check size
ls -lh build/bin/shadps4-pkg-extractor
```

## Packaging

### Create Portable Archive

```bash
# Linux
cd build/bin
tar -czf shadps4-pkg-extractor-linux-x64-v1.0.1.tar.gz shadps4-pkg-extractor

# Windows (in build/bin/Release)
7z a shadps4-pkg-extractor-windows-x64-v1.0.1.zip shadps4-pkg-extractor.exe
```

## Related

- [Usage Guide](README.md)
- [Main Build Instructions](../../BUILD.md)
- [PKG Format Documentation](../../docs/pkg-format.md)

---

**Build System**: CMake  
**Dependencies**: Crypto++, zlib (included)  
**Qt Required**: No
