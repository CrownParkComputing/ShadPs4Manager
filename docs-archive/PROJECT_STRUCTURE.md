# ShadPs4Manager Project Structure

## Overview
ShadPs4Manager is a comprehensive PS4 package management suite consisting of three applications:
1. **shadps4-manager-gui** - Main GUI application for managing PS4 games
2. **shadps4-pkg-extractor** - CLI tool for extracting PKG files
3. **shadps4-unlock-code-generator** - Admin tool for generating license keys

## Directory Structure

```
ShadPs4Manager/
├── build/                          # Build artifacts (gitignored)
│   ├── bin/                        # Compiled executables
│   │   ├── shadps4-manager-gui
│   │   ├── shadps4-pkg-extractor
│   │   └── shadps4-unlock-code-generator
│   └── lib/                        # Compiled libraries
│
├── docs/                           # GitHub Pages website
│   ├── index.html                  # Main website
│   ├── download.html               # Download page
│   └── assets/                     # Website assets
│
├── external/                       # External dependencies
│   ├── cryptopp/                   # Crypto++ library
│   ├── cryptopp-cmake/             # CMake wrapper for Crypto++
│   └── zlib/                       # zlib compression library
│
├── src/                            # Source code
│   ├── manager-gui/                # Main GUI application
│   │   ├── working_gui.cpp         # Main window
│   │   ├── settings.cpp/h          # Settings management
│   │   ├── settings_page.cpp/h     # Settings UI
│   │   ├── game_library.cpp/h      # Game library tab
│   │   ├── downloads_folder.cpp/h  # Downloads management
│   │   ├── installation_folder.cpp/h # Installed games view
│   │   ├── welcome_tab.cpp/h       # Registration/welcome screen
│   │   ├── gamecard.cpp/h          # Game card widget
│   │   ├── param_sfo.cpp/h         # PARAM.SFO parser
│   │   ├── igdbservice.cpp/h       # IGDB API integration
│   │   ├── credential_manager.cpp/h # Encrypted credentials
│   │   └── common_types.h          # Shared type definitions
│   │
│   ├── unlock-generator/           # License key generator
│   │   └── unlock_code_generator.cpp # Admin tool for generating codes
│   │
│   ├── pkg-extractor/              # CLI PKG extractor
│   │   └── pkg_extractor.cpp       # Command-line extraction tool
│   │
│   ├── common/                     # Shared common utilities
│   │   ├── arch.c                  # Architecture detection
│   │   ├── io_file.cpp/h           # File I/O utilities
│   │   ├── elf_info.h              # ELF file information
│   │   ├── logging.h               # Logging utilities
│   │   ├── scm_rev.cpp.in          # SCM revision template
│   │   └── types.h                 # Common type definitions
│   │
│   ├── core/                       # Core PKG handling
│   │   ├── crypto/                 # Cryptography
│   │   │   └── crypto.cpp/h        # PKG decryption
│   │   ├── file_format/            # File format handlers
│   │   │   ├── pkg.cpp/h           # PKG file format
│   │   │   ├── pkg_type.cpp/h      # PKG type detection
│   │   │   └── pkg_optimized.cpp/h # Optimized PKG handling
│   │   └── update_merger.cpp/h     # Update/patch merging
│   │
│   ├── pkg_tool/                   # PKG tool library
│   │   └── lib.cpp/h               # PKG extraction library
│   │
│   └── jukebox/                    # Audio files
│       └── *.mp3                   # Background music
│
├── scripts/                        # Build and utility scripts
│   ├── build.sh                    # Quick build script
│   └── clean.sh                    # Clean build artifacts
│
├── CMakeLists.txt                  # Main CMake configuration
├── VERSION                         # Version file
├── LICENSE                         # GPL-3.0 License
├── README.md                       # Project README
├── BUILD.md                        # Build instructions
├── REGISTRATION_SYSTEM.md          # License system documentation
├── ADMIN_GUIDE.md                  # Admin tool guide
├── RECORD_KEEPING.md               # Record keeping documentation
└── PROJECT_STRUCTURE.md            # This file
```

## Application Breakdown

### 1. ShadPs4 Manager GUI (shadps4-manager-gui)
**Purpose**: Main application for managing PS4 game packages

**Components**:
- **Game Library**: Display and launch installed games
- **Downloads Folder**: Scan and manage PKG files
- **Installed Games**: View installed content (games, updates, DLC)
- **Extraction Log**: Monitor PKG extraction progress
- **Settings**: Configure paths and API credentials
- **Welcome Tab**: Registration for trial users

**Dependencies**:
- Qt6 (Widgets, Network, Multimedia)
- shadps4_manager_core library
- Crypto++ (via core library)

**Size**: ~3.8 MB

### 2. PKG Extractor CLI (shadps4-pkg-extractor)
**Purpose**: Command-line tool for extracting PS4 PKG files

**Features**:
- Extract PKG files to directories
- Support for base games, updates, and DLC
- Verbose output mode
- Automatic directory creation

**Dependencies**:
- shadps4_manager_core library
- Crypto++
- zlib

**Size**: ~3.0 MB

**Usage**:
```bash
./shadps4-pkg-extractor <input.pkg> <output_dir> [--verbose]
```

### 3. Unlock Code Generator (shadps4-unlock-code-generator)
**Purpose**: Admin tool for generating license keys

**Features**:
- Generate unlock codes from System IDs
- Record customer emails (encrypted)
- View generation history
- Persistent encrypted storage

**Dependencies**:
- Qt6 (Widgets only)
- No core library needed (standalone)

**Size**: ~85 KB

**Storage**: 
- Records stored in: `~/.local/share/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`

## Build System

### CMake Configuration
The project uses CMake 3.20+ with the following structure:

1. **Core Library** (`shadps4_manager_core`):
   - Static library containing PKG handling, crypto, and file I/O
   - Used by both GUI and CLI applications
   - Depends on: Crypto++, zlib

2. **GUI Application** (`shadps4-manager-gui`):
   - Main end-user application
   - Depends on: Qt6, shadps4_manager_core

3. **CLI Extractor** (`shadps4-pkg-extractor`):
   - Command-line tool
   - Depends on: shadps4_manager_core

4. **Admin Tool** (`shadps4-unlock-code-generator`):
   - Independent admin application
   - Depends on: Qt6 only (no core library)

### Build Targets

```bash
# Build everything
make all

# Build individual targets
make shadps4-manager-gui
make shadps4-pkg-extractor
make shadps4-unlock-code-generator

# Build core library only
make shadps4_manager_core
```

## Dependencies

### Required for All
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Crypto++ 8.9.0
- zlib

### Required for GUI Applications
- Qt6 6.2+
  - Qt6::Widgets
  - Qt6::Network
  - Qt6::Multimedia

### Optional
- OpenGL (for hardware acceleration)

## Installation Locations

### Linux
- **Binaries**: `/usr/local/bin/` or `~/bin/`
- **Manager Data**: `~/.local/share/ShadPs4/`
- **Settings**: `~/.config/ShadPs4/ShadPs4 Manager.conf`
- **Generator Records**: `~/.local/share/ShadPs4 Manager Unlock Code Generator/`

### Windows
- **Binaries**: `C:\Program Files\ShadPs4Manager\`
- **Manager Data**: `%APPDATA%\ShadPs4\`
- **Settings**: `%APPDATA%\ShadPs4\ShadPs4 Manager.ini`
- **Generator Records**: `%APPDATA%\ShadPs4 Manager Unlock Code Generator\`

### macOS
- **Binaries**: `/Applications/` or `~/Applications/`
- **Manager Data**: `~/Library/Application Support/ShadPs4/`
- **Settings**: `~/Library/Preferences/com.shadps4.ShadPs4Manager.plist`
- **Generator Records**: `~/Library/Application Support/ShadPs4 Manager Unlock Code Generator/`

## Development Workflow

### Building from Source
```bash
# Clone repository
git clone https://github.com/CrownParkComputing/ShadPs4Manager.git
cd ShadPs4Manager

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir -p build && cd build

# Configure
cmake ..

# Build (parallel)
make -j$(nproc)

# Executables will be in build/bin/
```

### Quick Build Script
```bash
./build.sh
```

### Clean Build
```bash
cd build
make clean
# Or delete build directory entirely
cd .. && rm -rf build && mkdir build
```

## Release Process

### Version Management
Version is stored in `VERSION` file and embedded at compile time.

### Creating a Release
1. Update `VERSION` file
2. Update `CHANGELOG.md`
3. Build all targets in Release mode
4. Test all three executables
5. Create GitHub release
6. Upload binaries to GitHub release
7. Update website download links

### Binary Naming Convention
- **Linux**: `shadps4-manager-gui-linux-x64-v1.0.1`
- **Windows**: `shadps4-manager-gui-windows-x64-v1.0.1.exe`
- **macOS**: `shadps4-manager-gui-macos-universal-v1.0.1`

## Code Organization Principles

### Separation of Concerns
- **manager-gui/**: User-facing GUI application code only
- **unlock-generator/**: Admin tool code only (standalone)
- **pkg-extractor/**: CLI tool code only
- **common/**: Platform utilities, file I/O, logging
- **core/**: PKG format, crypto, file handling (business logic)
- **pkg_tool/**: High-level PKG extraction API

### Dependency Flow
```
manager-gui ──┐
              ├──> shadps4_manager_core ──┐
pkg-extractor ┘                           ├──> Crypto++, zlib
                                          │
unlock-generator ──> Qt6 only (independent)
```

### Why Separate unlock-generator?
- **Smaller binary**: Only 85KB vs 3.8MB
- **No unnecessary deps**: Doesn't need PKG handling
- **Security**: Isolated from main app
- **Portability**: Easier to distribute to admins

## Future Improvements

### Planned
- [ ] Separate CLI extractor from core library dependencies
- [ ] Plugin system for game metadata providers
- [ ] Automated testing suite
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Installer packages (DEB, RPM, MSI, DMG)

### Considerations
- Split pkg_tool into its own library?
- WebAssembly build for browser-based extraction?
- Docker container for reproducible builds?

## Contributing

When contributing, please:
1. Keep application code separated by directory
2. Update this document when adding new directories
3. Follow existing code structure
4. Test all three applications before PR
5. Update relevant documentation

## License

GPL-3.0 - See LICENSE file for details
