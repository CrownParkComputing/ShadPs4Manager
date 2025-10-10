# ShadPs4Manager

> Complete PS4 Package Management Suite for [ShadPS4 Emulator](https://github.com/shadps4-emu/shadPS4)

[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg)]()
[![Qt](https://img.shields.io/badge/Qt-6.2%2B-green.svg)](https://www.qt.io/)

A comprehensive toolkit for managing PS4 PKG files designed to work seamlessly with the [ShadPS4 emulator](https://github.com/shadps4-emu/shadPS4). Extract, organize, and manage PS4 games with an intuitive GUI or powerful command-line tools.

## About ShadPS4

[ShadPS4](https://github.com/shadps4-emu/shadPS4) is an early PlayStation 4 emulator for Windows, Linux, and macOS written in C++. This manager application complements ShadPS4 by providing professional-grade tools for:
- Extracting PS4 PKG files
- Managing game libraries
- Organizing updates and DLC
- Preparing games for emulation

## Three Powerful Tools

### 1. 🖥️ ShadPs4 Manager GUI (3.8 MB)
**Full-featured graphical application for managing your PS4 game collection**

- **Game Library**: Browse games with cover art from IGDB
- **PKG Extraction**: Multi-threaded extraction with progress tracking
- **Game Organization**: Automatic handling of base games, updates, and DLC
- **IGDB Integration**: Automatic metadata and artwork fetching
- **Registration System**: 10-game trial, unlock for unlimited use
- **ShadPS4 Integration**: Launch games directly in the emulator

### 2. ⚙️ PKG Extractor CLI (3.0 MB)
**Fast command-line tool for automated extraction**

```bash
shadps4-pkg-extractor game.pkg /output/dir --verbose
```

- Scriptable and automatable
- Perfect for batch processing
- No GUI dependencies
- Supports all PKG types

### 3. 🔑 Unlock Code Generator (85 KB)
**Admin tool for generating license keys** *(Administrators only)*

- Generate unlock codes from System IDs
- Encrypted customer record keeping
- Persistent history tracking

## Quick Start

### Download

Get the latest release: **[v1.0.1](https://github.com/CrownParkComputing/ShadPs4Manager/releases/latest)**

- 🐧 Linux: `shadps4-manager-linux-x64-v1.0.1.tar.gz`
- 🪟 Windows: `shadps4-manager-windows-x64-v1.0.1.zip`
- 🍎 macOS: `shadps4-manager-macos-universal-v1.0.1.dmg`

### Installation

**Linux:**
```bash
tar -xzf shadps4-manager-linux-x64-v1.0.1.tar.gz
cd shadps4-manager
chmod +x shadps4-manager-gui
./shadps4-manager-gui
```

**Windows:**
1. Extract ZIP file
2. Run `shadps4-manager-gui.exe`

**macOS:**
1. Open DMG and drag to Applications
2. Right-click → Open (first time)

## Usage

### Setting Up

1. **Launch Manager GUI**
2. **Configure Settings** (⚙️ tab):
   - **Downloads Folder**: Where your PKG files are located
   - **Extraction Path**: Where to extract files (temp location)
   - **Installation Folder**: Final game storage (for ShadPS4)
   - **IGDB API** *(optional)*: Get free keys from [IGDB](https://www.igdb.com/api)

### Extracting Games

1. Place PKG files in Downloads Folder
2. Click **Downloads Folder** tab
3. Click **Scan Directory**
4. Select PKG(s) and click **Install Selected**
5. Monitor progress in **Extraction Log** tab

Extracted games are automatically organized:
```
Installation Folder/
├── CUSA12345_GameName/          # Base game
├── CUSA12345_GameName-UPDATE/   # Updates
└── DLC/
    └── CUSA12345_DLC1/          # DLC content
```

### Using with ShadPS4

After extraction, point ShadPS4 to your Installation Folder:

**ShadPS4 Setup:**
1. Launch ShadPS4 emulator
2. Settings → Game Directories
3. Add your Installation Folder path
4. ShadPS4 will detect all extracted games

**Launch Games:**
- From ShadPs4Manager: Click game in Library → Launch
- From ShadPS4: Select game from library

### CLI Extraction

For automation or scripting:

```bash
# Extract single PKG
./shadps4-pkg-extractor game.pkg /output/

# Batch extract
for pkg in *.pkg; do
    ./shadps4-pkg-extractor "$pkg" "/output/"
done

# Verbose output
./shadps4-pkg-extractor game.pkg /output/ --verbose
```

## Building from Source

### Prerequisites

**All Platforms:**
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Qt6 6.2+ (for GUI tools)

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake git qt6-base-dev qt6-multimedia-dev
```

**macOS:**
```bash
brew install cmake qt@6
xcode-select --install
```

**Windows:**
- Install Visual Studio 2022
- Install Qt6 from [qt.io](https://www.qt.io/download)

### Build Steps

```bash
# Clone repository
git clone https://github.com/CrownParkComputing/ShadPs4Manager.git
cd ShadPs4Manager

# Initialize submodules (Crypto++, zlib)
git submodule update --init --recursive

# Build all applications
./scripts/build.sh

# Or manually
mkdir build && cd build
cmake ..
make -j$(nproc)

# Executables in build/bin/
ls -lh build/bin/
```

### Build Individual Tools

```bash
cd build

# GUI only
make shadps4-manager-gui

# CLI only
make shadps4-pkg-extractor

# Admin tool only
make shadps4-unlock-code-generator
```

## Features

### Game Management
- ✅ Visual game library with IGDB cover art
- ✅ Automatic game/update/DLC organization
- ✅ Installation tracking and status
- ✅ Game metadata display
- ✅ Search and filter functionality

### PKG Handling
- ✅ Support for base games, updates, and DLC
- ✅ Multi-threaded extraction
- ✅ Progress tracking and logging
- ✅ Automatic directory creation
- ✅ Error handling and recovery

### IGDB Integration
- ✅ Automatic metadata fetching
- ✅ Cover art and screenshots
- ✅ Game descriptions and details
- ✅ Release dates and genres
- ✅ Custom API key support

### Registration System
- ✅ 10-game trial for evaluation
- ✅ Hardware-based System ID
- ✅ Secure license activation
- ✅ PayPal payment integration
- ✅ No time limits

## System Requirements

### Minimum
- **OS**: Linux (Ubuntu 22.04+), Windows 10/11, macOS 11+
- **CPU**: Dual-core 2.0 GHz
- **RAM**: 2 GB
- **Disk**: 100 MB + game storage space

### Recommended
- **OS**: Latest stable version
- **CPU**: Quad-core 3.0 GHz+
- **RAM**: 4 GB+
- **Disk**: SSD with ample space
- **GPU**: For ShadPS4 emulation

## Project Structure

```
ShadPs4Manager/
├── src/
│   ├── manager-gui/          # Main GUI application
│   ├── pkg-extractor/        # CLI extraction tool
│   ├── unlock-generator/     # Admin license tool
│   ├── common/               # Shared utilities
│   ├── core/                 # PKG format handling
│   └── pkg_tool/             # PKG extraction library
│
├── scripts/
│   ├── build.sh              # Build script
│   └── clean.sh              # Clean build artifacts
│
├── docs/
│   ├── index.html            # GitHub Pages website
│   └── assets/               # Website resources
│
├── external/                 # Dependencies (submodules)
│   ├── cryptopp/             # Crypto++ library
│   └── zlib/                 # Compression library
│
├── CMakeLists.txt            # Build configuration
├── LICENSE                   # GPL-3.0 License
└── README.md                 # This file
```

## Troubleshooting

### Application Won't Launch

**Linux:**
```bash
# Check Qt6 installation
ldd shadps4-manager-gui | grep "not found"

# Install missing dependencies
sudo apt install qt6-base-dev qt6-multimedia-dev
```

**Windows:**
- Install Visual C++ Redistributables
- Ensure Qt6 DLLs are in same directory

**macOS:**
- Right-click → Open (bypass Gatekeeper)
- Requires macOS 11+

### PKG Files Not Detected

1. Verify Downloads Folder path in Settings
2. Ensure files have `.pkg` extension (lowercase)
3. Click "Scan Directory" to refresh
4. Check file permissions (must be readable)

### Extraction Fails

**Common causes:**
- Insufficient disk space (need 2x PKG size)
- Write permissions denied
- Corrupted PKG file
- Unsupported PKG type

**Solutions:**
```bash
# Check disk space
df -h /output/path

# Check permissions
ls -ld /output/path
chmod 755 /output/path

# Verify PKG integrity
md5sum file.pkg
```

### IGDB Not Working

1. Verify API credentials in Settings
2. Check internet connection
3. Get new credentials from [IGDB API](https://api-docs.igdb.com/)
4. Credentials must not be expired

## Registration & Licensing

### Trial Mode
- Install up to **10 items** (games + updates + DLC)
- All features fully functional
- No time limit
- Perfect for evaluation

### Full Version
1. Click PayPal button in Welcome tab
2. Complete payment
3. Email System ID to support
4. Receive unlock code within 24 hours
5. Enter code → Unlimited installations

**System ID**: Unique hardware identifier shown in Welcome tab

## Command-Line Reference

### PKG Extractor

```bash
# Basic usage
shadps4-pkg-extractor <input.pkg> <output_dir>

# Options
--verbose, -v     Enable detailed output
--quiet, -q       Suppress non-error output
--help, -h        Show help message
--version         Show version info

# Examples
shadps4-pkg-extractor game.pkg /games/
shadps4-pkg-extractor game.pkg /games/ --verbose

# Batch processing
for pkg in *.pkg; do
    shadps4-pkg-extractor "$pkg" "/games/$(basename "$pkg" .pkg)"
done

# With error handling
if shadps4-pkg-extractor game.pkg /games/; then
    echo "Success"
else
    echo "Failed with code $?"
fi
```

### Return Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Invalid arguments |
| 2 | File not found |
| 3 | Extraction failed |
| 4 | Insufficient disk space |
| 5 | Permission denied |

## Keyboard Shortcuts

- `Ctrl+S` - Open Settings
- `Ctrl+R` - Refresh/Scan
- `Ctrl+Q` - Quit
- `Ctrl+L` - Game Library
- `Ctrl+D` - Downloads Folder

## Integration with ShadPS4

### Workflow

1. **Extract with ShadPs4Manager**
   - Add PKG files to Downloads Folder
   - Extract to Installation Folder
   
2. **Configure ShadPS4**
   - Point ShadPS4 to Installation Folder
   - Games appear in ShadPS4 library

3. **Launch and Play**
   - Double-click game in ShadPS4
   - Or use Manager GUI launch button

### Directory Structure

Manager organizes games perfectly for ShadPS4:

```
Installation/
├── CUSA12345_GameTitle/
│   ├── app/
│   ├── param.sfo
│   └── icon0.png
├── CUSA12345_GameTitle-UPDATE/
│   └── patch files
└── DLC/
    └── CUSA12345_DLC/
        └── DLC content
```

### Best Practices

1. **Dedicated folder**: Create separate folder for ShadPS4 games
2. **SSD recommended**: Better performance for extraction and emulation
3. **Install order**: Base game → Updates → DLC
4. **Backup PKGs**: Keep original PKG files until verified working
5. **Regular updates**: Check for ShadPS4 and Manager updates

## FAQ

**Q: What is ShadPS4?**  
A: ShadPS4 is a PlayStation 4 emulator. This manager helps prepare games for it.

**Q: Is this safe?**  
A: Yes, it's open source (GPL-3.0). Review the code on GitHub.

**Q: Does this include games?**  
A: No. You must provide your own legally obtained PKG files.

**Q: Do I need ShadPS4 installed?**  
A: No, Manager works standalone. ShadPS4 is optional for actually playing games.

**Q: Can I use this on Steam Deck?**  
A: Yes! Runs on any Linux system with Qt6.

**Q: How do I get IGDB API keys?**  
A: Free at [IGDB.com](https://www.igdb.com/api). Optional feature.

**Q: What's the trial limit?**  
A: 10 items total (games + updates + DLC). After that, registration required.

**Q: Can I transfer my license?**  
A: Contact support for hardware changes with proof of purchase.

## Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create feature branch (`git checkout -b feature/awesome-feature`)
3. Commit changes (`git commit -m 'Add awesome feature'`)
4. Push to branch (`git push origin feature/awesome-feature`)
5. Open Pull Request

### Guidelines
- Follow existing code style
- Test all three applications
- Update documentation
- Keep commits focused

## License

**GNU General Public License v3.0 (GPL-3.0)**

You are free to:
- Use for any purpose
- Study and modify source code
- Distribute copies
- Distribute modified versions

See [LICENSE](LICENSE) for full terms.

## Support & Links

- **Website**: [https://crownparkcomputing.github.io/ShadPs4Manager/](https://crownparkcomputing.github.io/ShadPs4Manager/)
- **GitHub**: [https://github.com/CrownParkComputing/ShadPs4Manager](https://github.com/CrownParkComputing/ShadPs4Manager)
- **Issues**: [Report bugs](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- **ShadPS4**: [Official Emulator](https://github.com/shadps4-emu/shadPS4)

## Acknowledgments

- **[ShadPS4 Team](https://github.com/shadps4-emu/shadPS4)** - For the amazing PS4 emulator
- **Qt Framework** - Excellent GUI toolkit
- **Crypto++** - Cryptographic functions
- **IGDB** - Game metadata API
- **Contributors** - Everyone who helped this project

## Changelog

**v1.0.1** (2025-10-10)
- Initial public release
- Three applications: GUI, CLI, Admin tool
- Registration system with 10-game trial
- IGDB integration for metadata
- ShadPS4 integration and workflow
- Complete documentation and website

---

<p align="center">
  <strong>Made for the ShadPS4 community</strong><br>
  <a href="https://github.com/CrownParkComputing">Crown Park Computing</a>
</p>

<p align="center">
  <a href="https://github.com/CrownParkComputing/ShadPs4Manager">GitHub</a> •
  <a href="https://github.com/CrownParkComputing/ShadPs4Manager/releases">Releases</a> •
  <a href="https://github.com/shadps4-emu/shadPS4">ShadPS4 Emulator</a> •
  <a href="https://crownparkcomputing.github.io/ShadPs4Manager/">Website</a>
</p>
