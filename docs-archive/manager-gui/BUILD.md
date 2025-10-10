# Building ShadPs4 Manager GUI

This guide covers building the Manager GUI application from source.

## Prerequisites

### All Platforms

**Build Tools:**
- CMake 3.20 or higher
- C++20 compatible compiler:
  - GCC 11+ (Linux)
  - Clang 14+ (macOS)
  - MSVC 2022+ (Windows)
- Git (for cloning repository)

**Required Libraries:**
- Qt6 6.2+ (Widgets, Network, Multimedia)
- Crypto++ 8.9.0
- zlib

### Linux (Ubuntu/Debian)

```bash
# Build tools
sudo apt update
sudo apt install build-essential cmake git

# Qt6
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev

# Dependencies (included as submodules, but system versions work too)
sudo apt install libcrypto++-dev zlib1g-dev

# Optional: for multimedia support
sudo apt install libqt6multimedia6 qt6-multimedia-dev
```

### Linux (Fedora/RHEL)

```bash
# Build tools
sudo dnf install gcc-c++ cmake git

# Qt6
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel

# Dependencies
sudo dnf install cryptopp-devel zlib-devel
```

### Linux (Arch)

```bash
# Build tools
sudo pacman -S base-devel cmake git

# Qt6
sudo pacman -S qt6-base qt6-multimedia

# Dependencies
sudo pacman -S crypto++ zlib
```

### macOS

**Homebrew:**
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Build tools (Xcode Command Line Tools)
xcode-select --install

# CMake
brew install cmake

# Qt6
brew install qt@6

# Dependencies (included as submodules)
brew install cryptopp zlib
```

### Windows

**Visual Studio 2022:**
1. Download and install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/)
2. During installation, select:
   - Desktop development with C++
   - CMake tools for Windows
   - C++ Clang tools for Windows (optional)

**Qt6:**
1. Download [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. Install Qt 6.5 or later
3. Select components:
   - MSVC 2022 64-bit
   - Qt Multimedia
   - Qt Network

**CMake:**
- Included with Visual Studio, or download from [cmake.org](https://cmake.org/download/)

## Clone Repository

```bash
# Clone main repository
git clone https://github.com/CrownParkComputing/ShadPs4Manager.git
cd ShadPs4Manager

# Initialize submodules (Crypto++, zlib)
git submodule update --init --recursive
```

## Build Instructions

### Linux / macOS

**Quick Build:**
```bash
# From repository root
./build.sh
```

**Manual Build:**
```bash
# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build (parallel)
make -j$(nproc)

# Binary location
ls -lh bin/shadps4-manager-gui
```

**Build only Manager GUI:**
```bash
cd build
make shadps4-manager-gui -j$(nproc)
```

**Install system-wide (optional):**
```bash
cd build
sudo make install

# Or manually
sudo cp bin/shadps4-manager-gui /usr/local/bin/
```

### Windows (Visual Studio)

**Using CMake GUI:**
1. Open CMake GUI
2. Set source directory: `C:\path\to\ShadPs4Manager`
3. Set build directory: `C:\path\to\ShadPs4Manager\build`
4. Click "Configure"
   - Select "Visual Studio 17 2022"
   - Select "x64" platform
5. Set `CMAKE_PREFIX_PATH` to Qt installation:
   ```
   C:\Qt\6.5.0\msvc2022_64
   ```
6. Click "Generate"
7. Click "Open Project"
8. Build in Visual Studio (F7 or Build → Build Solution)

**Using Command Line:**
```cmd
:: Open "x64 Native Tools Command Prompt for VS 2022"
cd C:\path\to\ShadPs4Manager

:: Create build directory
mkdir build
cd build

:: Configure (adjust Qt path)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"

:: Build
cmake --build . --config Release

:: Binary location
dir bin\Release\shadps4-manager-gui.exe
```

### Windows (MinGW)

```bash
# Using MSYS2/MinGW64
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-qt6

cd /path/to/ShadPs4Manager
mkdir build && cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="/mingw64"
cmake --build . -j$(nproc)
```

## CMake Options

### Qt6 Path

If Qt6 is not found automatically:

```bash
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt6"
```

**Common Qt6 locations:**
- Linux: `/usr/lib/x86_64-linux-gnu/cmake/Qt6`
- macOS: `/opt/homebrew/opt/qt@6`
- Windows: `C:\Qt\6.5.0\msvc2022_64`

### Build Type

```bash
# Debug build (with symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### Install Prefix

```bash
# Install to custom location
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/shadps4

# Then install
make install
```

### Verbose Build

```bash
# See full compiler commands
make VERBOSE=1
```

## Build Output

Successful build creates:

```
build/
├── bin/
│   └── shadps4-manager-gui    # Main executable
└── lib/
    └── libshadps4_manager_core.a    # Core library
```

## Testing the Build

```bash
cd build/bin

# Run application
./shadps4-manager-gui

# Check dependencies (Linux)
ldd shadps4-manager-gui

# Check dependencies (macOS)
otool -L shadps4-manager-gui
```

## Common Build Issues

### Qt6 Not Found

**Error:**
```
CMake Error: Could not find Qt6
```

**Solution:**
```bash
# Linux: Install Qt6
sudo apt install qt6-base-dev qt6-multimedia-dev

# macOS: Install via Homebrew
brew install qt@6

# Set CMAKE_PREFIX_PATH
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt6"
```

### Crypto++ Not Found

**Error:**
```
Could not find cryptopp
```

**Solution:**
```bash
# Ensure submodules are initialized
git submodule update --init --recursive

# Or install system package
sudo apt install libcrypto++-dev    # Debian/Ubuntu
brew install cryptopp                # macOS
```

### C++20 Not Supported

**Error:**
```
error: C++20 is required
```

**Solution:**
```bash
# Update compiler
sudo apt install gcc-11 g++-11      # Ubuntu
brew install gcc@11                  # macOS

# Specify compiler
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### Missing Multimedia

**Error:**
```
Qt6Multimedia not found
```

**Solution:**
```bash
# Linux
sudo apt install qt6-multimedia-dev

# macOS
brew install qt@6    # Includes multimedia

# Windows: Reinstall Qt with Multimedia component
```

### Linker Errors

**Error:**
```
undefined reference to Qt6::...
```

**Solution:**
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Advanced Build Options

### Static Build

```bash
# Build with static Qt (requires static Qt build)
cmake .. -DBUILD_SHARED_LIBS=OFF
```

### Custom Optimization

```bash
# Maximum optimization
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"

# Size optimization
cmake .. -DCMAKE_CXX_FLAGS="-Os -s"
```

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Run with debugger
gdb build/bin/shadps4-manager-gui
```

### Sanitizers

```bash
# Address sanitizer (memory errors)
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address"

# Thread sanitizer (race conditions)
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=thread"

# Undefined behavior sanitizer
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=undefined"
```

## Cross-Compilation

### Linux → Windows

```bash
# Using MinGW cross-compiler
sudo apt install mingw-w64

mkdir build-windows && cd build-windows

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
  -DCMAKE_PREFIX_PATH="/path/to/qt6-mingw"

make -j$(nproc)
```

### macOS Universal Binary

```bash
# Build for both Intel and Apple Silicon
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
make -j$(nproc)
```

## Packaging

### Linux AppImage

```bash
# After building
cd build

# Create AppDir
make install DESTDIR=AppDir

# Download linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

# Create AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage
```

### macOS DMG

```bash
# After building
cd build

# Create app bundle
make install

# Create DMG
hdiutil create -volname "ShadPs4 Manager" \
  -srcfolder bin/shadps4-manager-gui.app \
  -ov -format UDZO \
  ShadPs4Manager.dmg
```

### Windows Installer

```bash
# Using NSIS
cd build
cpack -G NSIS
```

## Development Build

For active development:

```bash
# Configure once
mkdir build-dev && cd build-dev
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Rebuild quickly after changes
make shadps4-manager-gui -j$(nproc)

# Run
./bin/shadps4-manager-gui
```

## Clean Build

```bash
# Remove build artifacts
cd build
make clean

# Or completely remove build directory
cd ..
rm -rf build

# Then rebuild
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Build Time

Typical build times (parallel build on 8-core CPU):

- **Full build**: ~5-10 minutes
- **Incremental**: ~30 seconds
- **Clean rebuild**: ~7-12 minutes

## Next Steps

After building:
1. Test the application: `./build/bin/shadps4-manager-gui`
2. Check [README.md](README.md) for usage instructions
3. Review [USER_GUIDE.md](USER_GUIDE.md) for detailed features

## Support

Build issues? Check:
- [GitHub Issues](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- [Main BUILD.md](../../BUILD.md) for general build information
- [CMakeLists.txt](../../CMakeLists.txt) for build configuration

---

**Build System**: CMake 3.20+  
**License**: GPL-3.0
