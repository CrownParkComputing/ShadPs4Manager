# ShadPs4Manager

A powerful and user-friendly PS4 PKG file extractor with both GUI and command-line interfaces.

![Build Status](https://github.com/jon-crownpark/ShadPs4Manager/workflows/Build%20ShadPs4Manager/badge.svg)

## Features

- 🎯 **Dual Interface**: Modern Qt6 GUI and efficient command-line tool
- 📊 **Real-time Progress**: Live extraction progress with file-by-file tracking
- ⚡ **High Performance**: Optimized extraction with streaming decompression
- 🌍 **Cross-platform**: Windows, Linux, and macOS support
- 📱 **User-friendly**: Intuitive interface with detailed progress feedback
- 🔧 **Reliable**: Built on proven cryptographic libraries

## Screenshots

### GUI Application
The ShadPs4Manager GUI provides an intuitive interface for extracting PS4 PKG files:

- Real-time progress tracking
- Current file extraction display
- Extraction speed monitoring
- Detailed logging with timestamps
- Dark theme optimized for extended use

### Progress Tracking
- **File Progress**: Shows current file being extracted
- **Speed Monitoring**: Real-time extraction speed in MB/s
- **Size Tracking**: Total extracted size vs PKG file size
- **Time Tracking**: Elapsed time and estimated completion

## Installation

### Pre-built Binaries
Download the latest release from the [Releases](https://github.com/jon-crownpark/ShadPs4Manager/releases) page.

### Building from Source

#### Prerequisites
- **CMake** 3.16 or later
- **Qt6** (for GUI application)
- **C++20** compatible compiler
- **Git** (for cloning)

#### Build Instructions

```bash
# Clone the repository
git clone https://github.com/jon-crownpark/ShadPs4Manager.git
cd ShadPs4Manager

# Create build directory
mkdir build && cd build

# Configure (with GUI)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Configure (CLI only, no Qt6 required)
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF

# Build
cmake --build . --config Release

# Optional: Install
cmake --install . --config Release
```

#### Platform-specific Notes

**Linux:**
```bash
# Install Qt6 on Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Install Qt6 on Fedora
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

# Install Qt6 on Arch Linux
sudo pacman -S qt6-base qt6-tools cmake base-devel
```

**Windows:**
- Install Visual Studio 2022 with C++ support
- Install Qt6 from the official installer
- Use CMake GUI or command line with Visual Studio generator

**macOS:**
```bash
# Install dependencies via Homebrew
brew install qt6 cmake ninja
```

## Usage

### GUI Application

1. Launch `shadps4-manager-gui`
2. Click "Select PKG File" to choose your PS4 PKG file
3. Optionally select an output directory (defaults to same location as PKG)
4. Click "Extract PKG" to start extraction
5. Monitor real-time progress with file names and extraction speed

### Command Line Interface

```bash
# Extract to same directory as PKG file
./shadps4-cli /path/to/game.pkg

# Extract to specific directory
./shadps4-cli /path/to/game.pkg /path/to/output/directory

# Show help
./shadps4-cli --help
```

### Example Output

```
=== ShadPS4Manager PKG Extraction ===
PKG File: /home/user/Games/MyGame.pkg
Output Directory: /home/user/Games/MyGame_extracted
PKG Size: 14.2 GB

Starting extraction...
Extracted 45 files, 2.3 GB (Speed: 15.2 MB/s) (Elapsed: 02:15)
Current file: USRDIR/EBOOT.BIN
✅ Extraction completed successfully!
```

## Supported Formats

- **PS4 PKG Files**: All standard PS4 PKG archives
- **Compression**: PFSC compressed content
- **Encryption**: Standard PS4 PKG encryption schemes

## Technical Details

### Dependencies
- **cryptopp**: Cryptographic operations (RSA, AES, HMAC)
- **zlib**: Decompression support
- **Qt6**: GUI framework (optional)

### Architecture
- **Core Library**: Platform-independent extraction engine
- **CLI Tool**: Lightweight command-line interface
- **GUI Application**: Full-featured Qt6 interface
- **Cross-platform**: CMake-based build system

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Setup

```bash
git clone https://github.com/jon-crownpark/ShadPs4Manager.git
cd ShadPs4Manager

# Build in debug mode
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run tests
ctest
```

## License

This project is licensed under the GPL-2.0 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built upon the foundations of PS4 reverse engineering research
- Uses industry-standard cryptographic libraries
- Inspired by the need for reliable PS4 content extraction tools

## Troubleshooting

### Common Issues

**"CLI tool not found" error:**
- Ensure the CLI tool is built and accessible
- Check that both GUI and CLI are in the same directory

**Qt6 not found during build:**
- Install Qt6 development packages for your system
- Set Qt6_DIR environment variable if needed
- Use `-DBUILD_GUI=OFF` to build CLI-only version

**Extraction fails:**
- Verify the PKG file is valid and not corrupted
- Ensure sufficient disk space for extraction
- Check write permissions in the output directory

### Getting Help

- Open an issue on GitHub for bugs or feature requests
- Check existing issues for similar problems
- Provide detailed information including OS, Qt version, and error messages

## Changelog

### Version 1.0.0
- Initial release
- Full PS4 PKG extraction support
- Qt6 GUI with real-time progress tracking
- Command-line interface
- Cross-platform support (Windows, Linux, macOS)
- GitHub Actions CI/CD pipeline