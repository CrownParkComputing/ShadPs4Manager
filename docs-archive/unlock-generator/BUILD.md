# Building Unlock Code Generator

Build instructions for the admin unlock code generator tool.

## Quick Build

```bash
# From repository root
./build.sh

# Binary location
ls -lh build/bin/shadps4-unlock-code-generator
```

## Prerequisites

### All Platforms
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Qt6 6.2+ (Widgets only)

**Note**: No Crypto++ or zlib needed for this tool.

### Linux

```bash
sudo apt install build-essential cmake git qt6-base-dev
```

### macOS

```bash
xcode-select --install
brew install cmake qt@6
```

### Windows

1. Install Visual Studio 2022
2. Install Qt6 from [qt.io](https://www.qt.io/download)

## Build Steps

### Linux / macOS

```bash
# Clone repository
git clone https://github.com/CrownParkComputing/ShadPs4Manager.git
cd ShadPs4Manager

# Build
mkdir -p build && cd build
cmake ..
make shadps4-unlock-code-generator -j$(nproc)

# Test
./bin/shadps4-unlock-code-generator
```

### Windows

```cmd
cd C:\path\to\ShadPs4Manager
mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"

cmake --build . --config Release --target shadps4-unlock-code-generator

bin\Release\shadps4-unlock-code-generator.exe
```

## CMake Options

### Specify Qt6 Path

```bash
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt6"
```

Common Qt6 locations:
- Linux: `/usr/lib/x86_64-linux-gnu/cmake/Qt6`
- macOS: `/opt/homebrew/opt/qt@6`
- Windows: `C:\Qt\6.5.0\msvc2022_64`

### Build Type

```bash
# Release (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Build Only This Tool

```bash
cmake .. -DBUILD_GUI=OFF -DBUILD_CLI=OFF
make shadps4-unlock-code-generator
```

## Dependencies

The unlock generator depends on:
- **Qt6::Widgets** only
- **Qt6::Core** (included with Widgets)

No other dependencies required.

## Testing

```bash
cd build/bin

# Launch application
./shadps4-unlock-code-generator

# Check binary size (should be very small)
ls -lh shadps4-unlock-code-generator

# Expected: ~85 KB

# Check Qt dependencies (Linux)
ldd shadps4-unlock-code-generator | grep Qt
```

## Installation

### Linux

```bash
# System-wide
sudo cp build/bin/shadps4-unlock-code-generator /usr/local/bin/
sudo chmod +x /usr/local/bin/shadps4-unlock-code-generator
```

### Windows

Copy executable to desired location. May need Qt6 DLLs in same directory:
- Qt6Core.dll
- Qt6Gui.dll
- Qt6Widgets.dll

Or use `windeployqt`:
```cmd
cd build\bin\Release
windeployqt shadps4-unlock-code-generator.exe
```

### macOS

```bash
# Create app bundle (optional)
macdeployqt build/bin/shadps4-unlock-code-generator.app

# Copy to Applications
cp -r build/bin/shadps4-unlock-code-generator.app /Applications/
```

## Troubleshooting

### Qt6 Not Found

```bash
# Linux
sudo apt install qt6-base-dev

# macOS
brew install qt@6

# Then specify path
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt6"
```

### Missing Widgets

```bash
# Ensure Qt6 Widgets is installed
# Linux
sudo apt install qt6-base-dev

# macOS (included with qt@6)
brew reinstall qt@6
```

### Application Won't Launch

**Linux:**
```bash
# Check Qt plugins
export QT_DEBUG_PLUGINS=1
./shadps4-unlock-code-generator
```

**Windows:**
- Ensure Qt6 DLLs are in same directory or system PATH
- Use windeployqt to package all dependencies

**macOS:**
- Use macdeployqt to bundle Qt frameworks
- Check codesigning if Gatekeeper blocks

## Packaging

### Linux AppImage

```bash
cd build
make install DESTDIR=AppDir

# Download linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

# Download Qt plugin
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage

# Create AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir \
  --plugin qt \
  --output appimage
```

### Windows Portable

```cmd
cd build\bin\Release

:: Copy executable
mkdir portable
copy shadps4-unlock-code-generator.exe portable\

:: Deploy Qt dependencies
windeployqt portable\shadps4-unlock-code-generator.exe

:: Create ZIP
7z a shadps4-unlock-code-generator-windows-x64-v1.0.1.zip portable\
```

### macOS DMG

```bash
cd build/bin

# Deploy Qt
macdeployqt shadps4-unlock-code-generator.app

# Create DMG
hdiutil create -volname "Unlock Code Generator" \
  -srcfolder shadps4-unlock-code-generator.app \
  -ov -format UDZO \
  ShadPs4UnlockGenerator.dmg
```

## Static Build (Smaller Binary)

If you have a static Qt build:

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF \
  -DQt6_DIR="/path/to/qt6/static/lib/cmake/Qt6"

make shadps4-unlock-code-generator

# Result: Single executable with no external Qt dependencies
strip build/bin/shadps4-unlock-code-generator
```

## Reducing Binary Size

```bash
# Strip symbols
strip build/bin/shadps4-unlock-code-generator

# Before: ~600 KB
# After: ~85 KB

# Verify functionality
./build/bin/shadps4-unlock-code-generator
```

## Cross-Compilation

### Linux → Windows

```bash
# Install MinGW and Qt6-MinGW
sudo apt install mingw-w64

# Download Qt6 for MinGW from qt.io
# Then build:
mkdir build-windows && cd build-windows

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
  -DCMAKE_PREFIX_PATH="/path/to/qt6-mingw" \
  -DBUILD_GUI=OFF -DBUILD_CLI=OFF

make shadps4-unlock-code-generator
```

## Development Build

For rapid development:

```bash
mkdir build-dev && cd build-dev

cmake .. -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Quick rebuild after changes
make shadps4-unlock-code-generator -j$(nproc)

# Run
./bin/shadps4-unlock-code-generator
```

## Source Files

The unlock generator consists of a single file:
```
src/unlock-generator/
└── unlock_code_generator.cpp
```

To modify functionality, edit this file and rebuild.

## Related

- [Usage Guide](README.md)
- [Admin Guide](../../ADMIN_GUIDE.md)
- [Record Keeping](../../RECORD_KEEPING.md)
- [Main Build Instructions](../../BUILD.md)

---

**Build System**: CMake  
**Dependencies**: Qt6 Widgets only  
**Binary Size**: ~85 KB (stripped)
