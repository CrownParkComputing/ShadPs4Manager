# ShadPs4Manager

A modern GUI application for managing PS4 games, built with Qt6 and CMake.

## Features

- **Game Library Management**: Browse and organize your PS4 game collection with support for base games, updates, and DLC
- **Game Cards View**: Modern card-based interface with game artwork and metadata from IGDB
- **Folder Structure Support**: Organized folder naming for base games, updates (GameName-UPDATE), and DLC (DLC/GameName-DLC)
- **Launch Emulator**: Direct launcher for ShadPS4 emulator with or without games
- **System ShadPS4 Support**: Option to use system-installed ShadPS4 instead of specifying a custom path
- **Param.sfo Editor**: Edit PS4 game metadata directly
- **PKG File Support**: Extract and handle PS4 package files
- **IGDB Integration**: Automatic game metadata and artwork fetching
- **Dark Theme**: Modern dark UI for comfortable use

## Prerequisites

Before building the project, ensure you have the following installed:

### Required Dependencies

- **CMake** (>= 3.16)
- **Qt6** development libraries
- **GCC/G++** compiler
- **Make** build system
- **pkg-config**

### Installing Dependencies on Linux

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install cmake qt6-base-dev qt6-tools-dev build-essential pkg-config
```

#### Fedora/CentOS:
```bash
sudo dnf install cmake qt6-qtbase-devel qt6-qttools-devel gcc-c++ make pkg-config
```

#### Arch Linux:
```bash
sudo pacman -S cmake qt6-base qt6-tools base-devel pkg-config
```

## Building the Project

The project includes a convenient build script (`build.sh`) that handles all build operations:

### Quick Start

```bash
# Interactive Menu Mode (Recommended)
./build.sh

# Build the project
./build.sh build

# Build and run the application
./build.sh run

# Clean and rebuild everything
./build.sh rebuild

# Show all available options
./build.sh help
```

### Available Build Commands

| Command | Description |
|---------|-------------|
| `./build.sh` | Build the project (default) |
| `./build.sh build` | Build the project |
| `./build.sh run` | Build and run the application |
| `./build.sh clean` | Remove all build artifacts |
| `./build.sh configure` | Configure CMake build system |
| `./build.sh install` | Install the application system-wide |
| `./build.sh rebuild` | Clean and rebuild everything |
| `./build.sh help` | Show help information |

### Manual Build (Alternative)

If you prefer to build manually:

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON ..

# Build the project
make -j$(nproc)

# Run the application
./bin/shadps4-manager-gui
```

## Usage

1. **Launch the Application**:
   ```bash
   ./build.sh run
   ```

2. **Configure Settings**:
   - Set your game library path
   - Configure ShadPS4 executable location:
     - **Option 1**: Browse to a specific ShadPS4 executable
     - **Option 2**: Check "Use System-Installed ShadPS4" to use the version in your PATH
   - Set up DLC folder path
   - Configure IGDB API credentials (optional, for game metadata)

3. **Game Library Organization**:
   Organize your games following this structure:
   ```
   GameLibrary/
   ├── GameName/              # Base game
   ├── GameName-UPDATE/       # Game updates
   └── DLC/
       ├── GameName-DLC/      # DLC for GameName
       └── GameName-DLC-2/    # Additional DLC
   ```
   See [FOLDER_STRUCTURE.md](FOLDER_STRUCTURE.md) for detailed information.

4. **Launch Options**:
   - **Launch Game**: Click on a game card to launch it with ShadPS4
   - **Launch Emulator Only**: Click "Launch Emulator" button to start ShadPS4 without a specific game

5. **Game Library Features**:
   - Browse your PS4 games in a modern card-based interface
   - View game information and metadata from IGDB
   - Launch games directly with detected updates and DLC
   - Edit param.sfo files for each game

6. **Downloads Management**:
   - Extract PKG files to your game library
   - Automatic detection of base games, updates, and DLC packages

## Project Structure

```
ShadPs4Manager/
├── src/
│   ├── gui/           # GUI application source files
│   ├── core/          # Core library functionality
│   ├── pkg_tool/      # PKG file handling
│   └── common/        # Shared utilities
├── external/          # Third-party libraries
├── build.sh           # Build script
├── CMakeLists.txt     # CMake configuration
└── README.md          # This file
```

## Troubleshooting

### Common Issues

1. **Qt6 not found**:
   - Make sure Qt6 development packages are installed
   - Check that `pkg-config --exists Qt6Widgets` returns successfully

2. **Build errors**:
   - Run `./build.sh clean` to clean build artifacts
   - Run `./build.sh rebuild` to rebuild from scratch

3. **Runtime issues**:
   - Ensure ShadPS4 is properly installed and configured
   - Check file permissions for game directories

### Getting Help

- Run `./build.sh help` for build script options
- Check the [Issues](../../issues) page for known problems
- Create a new issue if you encounter problems

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## License

This project is licensed under the terms specified in the LICENSE file.
