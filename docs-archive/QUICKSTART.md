# Quick Start Guide

Welcome to **ShadPs4Manager**! This guide will help you get started in minutes.

## What is ShadPs4Manager?

ShadPs4Manager is a professional PS4 package management suite consisting of three applications:

1. **ShadPs4 Manager GUI** - Main application for managing PS4 games
2. **PKG Extractor CLI** - Command-line extraction tool
3. **Unlock Code Generator** - Admin tool for generating license keys

## Installation

### Download

Visit the [releases page](https://github.com/CrownParkComputing/ShadPs4Manager/releases/latest) and download the appropriate version for your platform:

- **Linux**: `shadps4-manager-linux-x64-v1.0.1.tar.gz`
- **Windows**: `shadps4-manager-windows-x64-v1.0.1.zip`
- **macOS**: `shadps4-manager-macos-universal-v1.0.1.dmg`

### Linux Installation

```bash
# Extract the archive
tar -xzf shadps4-manager-linux-x64-v1.0.1.tar.gz

# Make executables runnable
cd shadps4-manager
chmod +x shadps4-manager-gui
chmod +x shadps4-pkg-extractor
chmod +x shadps4-unlock-code-generator

# Run the main application
./shadps4-manager-gui
```

### Windows Installation

1. Extract the ZIP file
2. Double-click `shadps4-manager-gui.exe`
3. If Windows SmartScreen appears, click "More info" → "Run anyway"

### macOS Installation

1. Open the DMG file
2. Drag applications to your Applications folder
3. Right-click and select "Open" the first time (to bypass Gatekeeper)

## First Launch

### Welcome Screen

When you first launch ShadPs4 Manager GUI, you'll see a **Welcome** tab:

1. **System ID** - Your unique hardware ID (automatically generated)
2. **PayPal Payment** - Link to register the application
3. **License Activation** - Field to enter your unlock code

### Trial Mode

You can use ShadPs4Manager in trial mode with these limitations:
- Install up to **10 items** (games + updates + DLC combined)
- All features are fully functional
- No time limit

This gives you plenty of time to evaluate the software!

## Basic Usage

### 1. Configure Settings

Click the **Settings** tab (⚙️ icon) and configure:

**Paths:**
- **Downloads Folder** - Where your PKG files are located
- **Extraction Path** - Where games will be extracted
- **Installation Folder** - Where extracted games are stored

**IGDB Integration (Optional):**
- Get a free API key from [IGDB](https://www.igdb.com/api)
- Enter Client ID and Client Secret
- This enables game metadata and cover art

### 2. Scan Downloads Folder

1. Click the **Downloads Folder** tab
2. Click **Scan Directory**
3. Wait for PKG files to be discovered
4. You'll see a list of all PKG files with:
   - Title ID
   - Title
   - Version
   - Type (Game, Update, DLC)
   - Size

### 3. Extract a Game

1. In the **Downloads Folder** tab, select a PKG file
2. Click **Install Selected**
3. Watch the **Extraction Log** tab for progress
4. When complete, the game appears in **Game Library**

### 4. View Installed Games

Click the **Installation Folder** tab to see:
- All installed base games
- Available updates
- Installed DLC

### 5. Launch from Library

1. Click **Game Library** tab
2. Browse your games with cover art
3. Double-click to launch (if applicable)
4. Right-click for options

## Registration

### Why Register?

Registration removes the 10-item trial limit, allowing unlimited game installations.

### How to Register

1. Copy your **System ID** from the Welcome tab
2. Click the **Pay with PayPal** button
3. Complete payment via PayPal
4. Email your System ID to the provided address
5. Receive your unlock code within 24 hours
6. Enter the unlock code in the **License Activation** field
7. Click **Activate License**

Once registered, the Welcome tab disappears and you have full access!

## CLI Usage

### Basic Extraction

```bash
./shadps4-pkg-extractor game.pkg /output/directory
```

### Verbose Mode

```bash
./shadps4-pkg-extractor game.pkg /output/directory --verbose
```

### Batch Processing

```bash
for pkg in *.pkg; do
    ./shadps4-pkg-extractor "$pkg" "/output/$(basename "$pkg" .pkg)"
done
```

## Tips & Tricks

### Organizing PKG Files

Keep your PKG files organized:
```
Downloads/
├── Base Games/
│   ├── CUSA12345.pkg
│   └── CUSA67890.pkg
├── Updates/
│   ├── CUSA12345-update-v1.01.pkg
│   └── CUSA67890-update-v1.05.pkg
└── DLC/
    ├── CUSA12345-dlc1.pkg
    └── CUSA12345-dlc2.pkg
```

### Install Order

ShadPs4Manager automatically handles install order:
1. Base game must be installed first
2. Updates install on top of base game
3. DLC requires base game + latest update

### Keyboard Shortcuts

- **Ctrl+S** - Open Settings
- **Ctrl+R** - Refresh/Scan
- **Ctrl+Q** - Quit application

### Finding Game Metadata

If IGDB doesn't find your game:
1. Make sure the PKG file has proper metadata
2. Check your IGDB API credentials
3. Try searching by Title ID (e.g., CUSA12345)

## Troubleshooting

### "Permission Denied" Error

**Linux/macOS:**
```bash
chmod +x shadps4-manager-gui
```

### PKG Files Not Showing

1. Check Downloads Folder path in Settings
2. Ensure PKG files are in the correct location
3. Click "Scan Directory" to refresh
4. Check file permissions (must be readable)

### Extraction Fails

1. Check available disk space (need 2x PKG size)
2. Verify extraction path is writable
3. Check Extraction Log for specific errors
4. Try extracting to a different location

### IGDB Not Working

1. Verify your API credentials are correct
2. Check internet connection
3. Ensure credentials haven't expired
4. Get new credentials from [IGDB API](https://api-docs.igdb.com/)

### License Activation Fails

1. Double-check unlock code (case-sensitive)
2. Ensure System ID matches the one registered
3. Copy-paste instead of typing to avoid errors
4. Contact support if issues persist

## Getting Help

### Documentation

- **README.md** - Project overview
- **BUILD.md** - Build from source instructions
- **REGISTRATION_SYSTEM.md** - Technical details
- **ADMIN_GUIDE.md** - For unlock code generators
- **PROJECT_STRUCTURE.md** - Codebase organization

### Support Channels

- **GitHub Issues**: [Report bugs or request features](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- **Website**: [https://crownparkcomputing.github.io/ShadPs4Manager/](https://crownparkcomputing.github.io/ShadPs4Manager/)

### Common Questions

**Q: Is this safe to use?**  
A: Yes! ShadPs4Manager is open source (GPL-3.0). You can review the entire codebase.

**Q: Does this work on SteamDeck?**  
A: Yes! It runs on any Linux system with Qt6 installed.

**Q: Can I use this commercially?**  
A: Check the GPL-3.0 license. Personal use is always fine.

**Q: How do I update the software?**  
A: Download the latest release and replace the old executables.

**Q: Where are my settings stored?**  
- **Linux**: `~/.config/ShadPs4/ShadPs4 Manager.conf`
- **Windows**: `%APPDATA%\ShadPs4\ShadPs4 Manager.ini`
- **macOS**: `~/Library/Preferences/com.shadps4.ShadPs4Manager.plist`

## Advanced Features

### Command-Line Arguments

```bash
# Launch with specific settings file
./shadps4-manager-gui --config=/path/to/config.ini

# Verbose logging
./shadps4-manager-gui --verbose

# Headless extraction (future feature)
./shadps4-manager-gui --extract=/path/to/file.pkg --output=/path/to/output
```

### Environment Variables

```bash
# Override default paths
export SHADPS4_DOWNLOADS="/path/to/downloads"
export SHADPS4_EXTRACTION="/path/to/extraction"
./shadps4-manager-gui
```

### Batch Operations

Use the CLI tool for automated workflows:

```bash
#!/bin/bash
# Extract all PKGs in a directory

INPUT_DIR="/path/to/pkgs"
OUTPUT_DIR="/path/to/output"

for pkg in "$INPUT_DIR"/*.pkg; do
    echo "Extracting: $(basename "$pkg")"
    ./shadps4-pkg-extractor "$pkg" "$OUTPUT_DIR" --verbose
done

echo "All extractions complete!"
```

## Next Steps

1. **Configure your paths** in Settings
2. **Add your PKG files** to the Downloads folder
3. **Scan the directory** to discover games
4. **Extract your first game**
5. **Register when ready** to unlock full features

Enjoy using ShadPs4Manager! 🎮

---

**Project**: ShadPs4Manager  
**License**: GPL-3.0  
**Website**: https://crownparkcomputing.github.io/ShadPs4Manager/  
**GitHub**: https://github.com/CrownParkComputing/ShadPs4Manager
