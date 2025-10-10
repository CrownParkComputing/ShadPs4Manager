# Change Summary - New Features

## Overview
This update adds several new features to ShadPs4Manager to improve emulator integration and game organization.

## New Features

### 1. System ShadPS4 Support
**Location**: Settings Page

- Added a checkbox "Use System-Installed ShadPS4 (from PATH)"
- When enabled, the manager will automatically find `shadps4` from your system PATH
- No need to manually browse for the executable if it's installed system-wide
- Falls back to common installation paths if not found in PATH

**Implementation**:
- `src/gui/settings.h`: Added `getUseSystemShadPS4()` and `setUseSystemShadPS4()` methods
- `src/gui/settings.cpp`: Modified `getShadPS4Path()` to check system PATH when enabled
- `src/gui/settings_page.h/cpp`: Added checkbox UI element

### 2. Launch Emulator Only
**Location**: Main GUI Menu Bar

- Added "Launch Emulator" button in the main menu
- Launches ShadPS4 without loading any specific game
- Allows users to:
  - Access emulator's built-in game library
  - Configure emulator settings directly
  - Browse and select games from within the emulator

**Implementation**:
- `src/gui/working_gui.cpp`: 
  - Added `launchEmulator()` slot method
  - Added "Launch Emulator" button with purple styling
  - Connected button to launch functionality

### 3. Organized Folder Structure
**Location**: Game Library Detection

- Implemented support for structured game folders:
  - **Base Game**: `GameName/` - Main game folder
  - **Updates**: `GameName-UPDATE/` - Game patches and updates
  - **DLC**: `DLC/GameName-DLC/` - All DLC in dedicated subfolder

- Game cards now display:
  - Update status: "GameName (with updates)"
  - DLC count: "GameName [2 DLC]"

**Implementation**:
- `src/gui/game_library.cpp`: 
  - Modified `loadGames()` to filter UPDATE and DLC folders
  - Added detection for UPDATE folders by suffix match
  - Added detection for DLC folders in `DLC/` subdirectory
  - Updated game metadata to show update and DLC status

**Documentation**:
- Created `FOLDER_STRUCTURE.md` with detailed guide and examples

## Files Modified

### Core Settings
- `src/gui/settings.h` - Added system shadPS4 preference methods
- `src/gui/settings.cpp` - Implemented system PATH detection logic

### Settings Page UI
- `src/gui/settings_page.h` - Added checkbox member variable
- `src/gui/settings_page.cpp` - Added checkbox UI and connection

### Main GUI
- `src/gui/working_gui.cpp` - Added launch emulator button and functionality

### Game Library
- `src/gui/game_library.cpp` - Added folder structure detection

### Documentation
- `README.md` - Updated with new features
- `FOLDER_STRUCTURE.md` - New comprehensive folder structure guide

## Usage Examples

### Example 1: Using System ShadPS4
1. Install shadPS4 to `/usr/bin/shadps4` or add it to PATH
2. Open Settings
3. Check "Use System-Installed ShadPS4"
4. No need to browse for executable path

### Example 2: Launch Emulator Directly
1. Click "Launch Emulator" button in main menu
2. Emulator starts without loading any game
3. Use emulator's interface to select and configure games

### Example 3: Organized Game Folders
```
GameLibrary/
├── Bloodborne/                  # Base game shows as "Bloodborne"
├── Bloodborne-UPDATE/           # Shows as "Bloodborne (with updates)"
└── DLC/
    ├── Bloodborne-DLC/          # Shows as "Bloodborne (with updates) [2 DLC]"
    └── Bloodborne-DLC-2/
```

## Testing

The project has been successfully compiled with all changes:
- Build completed without errors
- All modified files compile cleanly
- Binary size: ~3.6MB for GUI application

## Backwards Compatibility

All changes are backwards compatible:
- Existing game libraries continue to work
- Custom shadPS4 path method still available
- Old folder structures are still detected
- No breaking changes to configuration files

## Future Enhancements

Potential future improvements:
- Automatic UPDATE/DLC folder detection during PKG extraction
- Merge update content into base game option
- DLC management interface
- Game version checking and update notifications

