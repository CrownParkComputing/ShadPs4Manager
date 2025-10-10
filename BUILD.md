# ShadPs4Manager Build System

This repository includes a comprehensive build system with version management and GitHub Actions for automated builds.

## ⚠️ IMPORTANT: Required Executables

**ShadPs4Manager requires TWO executables to function properly:**

1. **`shadps4-manager-gui`** - The main GUI application
2. **`shadps4-pkg-extractor`** - The CLI tool for PKG extraction

Both executables **MUST** be in the same folder for the application to work. The GUI will check for the CLI tool on startup and display an error if it's missing.

### Why Two Executables?

The application uses a **separation of concerns** architecture:
- **GUI** (`shadps4-manager-gui`) - User interface, game library, settings
- **CLI** (`shadps4-pkg-extractor`) - PKG file extraction, validation, processing

This design provides:
- **Process isolation** - CLI crashes don't affect the GUI
- **Better debugging** - Test CLI independently
- **Simpler threading** - Each extraction is a separate process
- **Easier maintenance** - Changes to extraction logic don't require GUI rebuilds

## Quick Start

### Local Building

```bash
# Simple build (Linux)
./build.sh

# This will create:
# - build/bin/shadps4-manager-gui      (Main application)
# - build/bin/shadps4-pkg-extractor    (PKG extraction tool)
```

After building, **both executables** will be in `build/bin/`. You can run the GUI with:

```bash
cd build/bin
./shadps4-manager-gui
```

The GUI will automatically look for `shadps4-pkg-extractor` in the same directory.

#### Advanced Build Options

```bash
# Build with version increment
./build.sh patch    # 1.0.0 -> 1.0.1
./build.sh minor    # 1.0.0 -> 1.1.0
./build.sh major    # 1.0.0 -> 2.0.0

# Show help
./build.sh help
```

### GitHub Actions

The repository includes three GitHub Actions workflows:

1. **Linux Build** (`.github/workflows/build-linux.yml`)
   - Triggers on push to main/develop branches
   - Builds for Ubuntu 22.04
   - Creates distribution packages
   - Uploads artifacts

2. **Windows Build** (`.github/workflows/build-windows.yml`)
   - Triggers on push to main/develop branches
   - Builds for Windows Server 2022
   - Includes Qt6 dependencies
   - Creates ZIP distributions

3. **Release Workflow** (`.github/workflows/release.yml`)
   - Manual trigger with version bump options
   - Builds for both Linux and Windows
   - Creates GitHub releases with assets
   - Automatically tags versions

## Build Script Features

### Version Management
- Automatic semantic versioning (major.minor.patch)
- Updates CMakeLists.txt and version templates
- Git commits and tags versions automatically
- Tracks version history

### Distribution Creation
- Creates platform-specific packages
- Includes binaries, documentation, and install scripts
- Generates tar.gz for Linux, ZIP for Windows
- Proper executable permissions

### Dependencies Detection
- Checks for CMake, Qt6, compilers
- Provides helpful error messages
- Validates build environment

## Usage Examples

### Development Workflow
```bash
# Make changes to code
git add .
git commit -m "Add new feature"

# Increment version and build
./build.sh patch

# Push changes (version bump already committed)
git push origin main
```

### Release Workflow
1. Navigate to GitHub Actions
2. Click "Release" workflow
3. Select "Run workflow"
4. Choose version bump type (patch/minor/major)
5. Enable "Create GitHub release"
6. Click "Run workflow"

This will:
- Bump version automatically
- Build for Linux and Windows
- Create a GitHub release with binaries
- Tag the version in git

## Directory Structure

```
ShadPs4Manager/
├── build.sh              # Main build script
├── VERSION               # Current version file
├── .github/
│   └── workflows/        # GitHub Actions
│       ├── build-linux.yml
│       ├── build-windows.yml
│       └── release.yml
├── build/               # Build output (local)
├── dist/                # Distribution packages
└── src/                 # Source code
```

## Build Outputs

### Linux Distribution
```
ShadPs4Manager-X.Y.Z-linux-x64.tar.gz
├── shadps4-manager-gui      # Main GUI application ⭐ REQUIRED
├── shadps4-pkg-extractor    # PKG extraction tool ⭐ REQUIRED
├── install.sh               # Installation script
├── README.md                # Documentation
└── LICENSE                  # License file
```

**Both executables must remain in the same folder!**

### Windows Distribution
```
ShadPs4Manager-X.Y.Z-windows-x64.zip
├── shadps4-manager-gui.exe     # Main GUI application ⭐ REQUIRED
├── shadps4-pkg-extractor.exe   # PKG extraction tool ⭐ REQUIRED
├── install.bat                 # Installation script
├── Qt6*.dll                    # Qt6 dependencies
├── README.md                   # Documentation
└── LICENSE                     # License file
```

**Both executables must remain in the same folder!**

## Requirements

### Linux
- CMake 3.16+
- Qt6 development libraries
- GCC or Clang compiler
- Git (for version management)

### Windows (GitHub Actions)
- Windows Server 2022
- MSVC 2019
- Qt6 (automatically installed)
- CMake (automatically available)

## Troubleshooting

### Common Issues

**"PKG Extractor not found" on startup**
```
Problem: GUI can't find shadps4-pkg-extractor
Solution: Ensure both executables are in the same folder:
  - shadps4-manager-gui
  - shadps4-pkg-extractor
  
You can also set a custom path in Settings > PKG Extractor Path
```

**"CMake not found"**
```bash
# Ubuntu/Debian
sudo apt install cmake

# Arch Linux
sudo pacman -S cmake
```

**"Qt6 not found"**
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev

# Arch Linux
sudo pacman -S qt6-base qt6-tools
```

**Build script permission error**
```bash
chmod +x build.sh
```

### GitHub Actions Issues

- Check that Qt6 installation succeeded
- Verify CMake configuration step
- Check artifact upload permissions
- Ensure GITHUB_TOKEN has sufficient permissions for releases

## Contributing

1. Fork the repository
2. Make your changes
3. Test locally with `./build.sh`
4. Create a pull request
5. GitHub Actions will automatically build and test

The build system will automatically handle version management and distribution creation for releases.