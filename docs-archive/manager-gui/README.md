# ShadPs4 Manager GUI

The main graphical user interface application for managing PS4 packages.

## Overview

ShadPs4 Manager GUI is a comprehensive Qt6-based application that provides:
- Visual game library with cover art
- Batch PKG extraction
- IGDB metadata integration
- Update and DLC management
- Registration system with trial mode
- Real-time extraction progress

**Size**: ~3.8 MB  
**License**: GPL-3.0

## Features

### Game Library
- Browse installed games with cover art
- View game details from IGDB
- Launch games (if supported)
- Sort and filter your collection

### PKG Management
- Scan directories for PKG files
- View PKG metadata (Title ID, version, type)
- Install games, updates, and DLC
- Track installation progress

### Registration System
- 10-game trial mode
- Hardware-based System ID
- PayPal integration for registration
- Secure license activation

### Settings
- Configure download and extraction paths
- IGDB API credentials
- Audio preferences
- Application customization

## System Requirements

### Minimum Requirements
- **OS**: Linux (Ubuntu 22.04+), Windows 10/11, macOS 11+
- **CPU**: Dual-core 2.0 GHz
- **RAM**: 2 GB
- **Disk**: 100 MB for application + space for games
- **Display**: 1280x720 minimum

### Recommended Requirements
- **OS**: Latest stable OS version
- **CPU**: Quad-core 3.0 GHz
- **RAM**: 4 GB
- **Disk**: SSD with ample space
- **Display**: 1920x1080 or higher

### Dependencies
- Qt6 (Widgets, Network, Multimedia)
- OpenGL (for hardware acceleration)
- Internet connection (for IGDB integration)

## Installation

### Linux

**From Binary:**
```bash
# Extract archive
tar -xzf shadps4-manager-gui-linux-x64-v1.0.1.tar.gz
cd shadps4-manager-gui

# Make executable
chmod +x shadps4-manager-gui

# Run
./shadps4-manager-gui
```

**System-wide installation:**
```bash
sudo cp shadps4-manager-gui /usr/local/bin/
sudo chmod +x /usr/local/bin/shadps4-manager-gui

# Run from anywhere
shadps4-manager-gui
```

**Desktop Entry (Optional):**
```bash
cat > ~/.local/share/applications/shadps4-manager.desktop <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=ShadPs4 Manager
Comment=PS4 Package Manager
Exec=/usr/local/bin/shadps4-manager-gui
Icon=utilities-terminal
Terminal=false
Categories=Game;Utility;
EOF
```

### Windows

1. Extract ZIP file
2. Double-click `shadps4-manager-gui.exe`
3. If SmartScreen appears: Click "More info" → "Run anyway"

**Optional**: Create shortcut on desktop

### macOS

1. Open DMG file
2. Drag to Applications folder
3. Right-click → "Open" (first launch only)

## First Launch

### Welcome Screen

On first launch, you'll see the **Welcome** tab:

1. **System ID**: Your unique hardware identifier
   - Automatically generated
   - Used for license registration
   - Copy this for registration

2. **PayPal Payment**: 
   - Click to open payment page
   - System ID shown as reminder
   - Complete payment to register

3. **License Activation**:
   - Enter unlock code received
   - Click "Activate License"
   - Welcome tab disappears on success

### Trial Mode

**Limitations:**
- Maximum 10 items (games + updates + DLC)
- All features fully functional
- No time limit

**Counting:**
- Base game = 1 item
- Update = 1 item
- DLC pack = 1 item

Example: Game + 2 updates + 3 DLC = 6 items

## Configuration

### Settings Tab

Click the **Settings** (⚙️) icon to configure:

#### Paths

**Downloads Folder**: Where PKG files are stored
```
Example: /home/user/Downloads/PS4-PKGs
```

**Extraction Path**: Temporary extraction location
```
Example: /home/user/.local/share/ShadPs4/temp
```

**Installation Folder**: Where games are installed
```
Example: /home/user/Games/PS4
```

#### IGDB Integration

Get free API credentials from [IGDB](https://www.igdb.com/api):

1. Create an account at IGDB
2. Register a new application
3. Copy Client ID and Client Secret
4. Paste into Settings
5. Click "Save Settings"

**Benefits:**
- Game cover art
- Metadata (release date, genre, etc.)
- Screenshots and videos
- Game descriptions

#### Audio Settings

- Background music on/off
- Volume control
- Select music tracks

### Keyboard Shortcuts

- `Ctrl+S` - Open Settings
- `Ctrl+R` - Refresh/Scan
- `Ctrl+Q` - Quit
- `Ctrl+L` - Open Game Library
- `Ctrl+D` - Open Downloads Folder

## Usage

### Adding PKG Files

1. Place PKG files in your Downloads Folder
2. Open Manager GUI
3. Click **Downloads Folder** tab
4. Click **Scan Directory**
5. PKGs appear in the list

### Installing a Game

**Method 1: From Downloads Folder**
1. Go to **Downloads Folder** tab
2. Select PKG file(s)
3. Click **Install Selected**
4. Monitor **Extraction Log** tab

**Method 2: Drag and Drop**
1. Drag PKG file into Downloads Folder tab
2. Confirm installation

### Install Order

Manager automatically handles installation order:

1. **Base Game First**: Must install before updates/DLC
2. **Updates**: Install after base game
3. **DLC**: Requires base game + updates

Example workflow:
```
1. Install: CUSA12345_base.pkg (base game)
2. Install: CUSA12345_patch_v1.01.pkg (update)
3. Install: CUSA12345_dlc1.pkg (DLC)
```

### Viewing Installed Games

**Game Library Tab:**
- Grid view with cover art
- Click for details
- Double-click to launch

**Installation Folder Tab:**
- Tree view of all content
- Shows base games, updates, DLC
- Update status indicators

### Updating Games

1. Add update PKG to Downloads Folder
2. Scan directory
3. Install update PKG
4. Manager merges with base game

### Managing DLC

1. Install base game first
2. Install any updates
3. Install DLC packages
4. DLC appears in Installation Folder

## Registration

### Why Register?

- Remove 10-item trial limit
- Support development
- One-time payment
- Lifetime license

### Registration Process

1. **Copy System ID** from Welcome tab
2. **Click PayPal button** to open payment page
3. **Complete payment** via PayPal
4. **Email System ID** to support (shown on payment page)
5. **Receive unlock code** within 24 hours
6. **Enter code** in License Activation field
7. **Click Activate** - Welcome tab disappears

### System ID

Your System ID is generated from:
- Machine UUID
- Hostname
- MAC addresses
- Hashed with SHA-256

**Important**: System ID changes if you:
- Change hardware
- Reinstall OS (sometimes)
- Change network adapter

Contact support for hardware change transfers.

## Troubleshooting

### Application Won't Launch

**Linux:**
```bash
# Check Qt6 installation
ldd shadps4-manager-gui | grep "not found"

# Install missing Qt6 packages
sudo apt install qt6-base-dev qt6-multimedia-dev
```

**Windows:**
- Ensure Visual C++ Redistributables are installed
- Try running as Administrator

**macOS:**
- Check Gatekeeper: Right-click → Open
- Verify macOS 11+ (Big Sur or later)

### PKG Files Not Detected

1. **Verify path**: Settings → Downloads Folder
2. **Check permissions**: Folder must be readable
3. **File extension**: Must be `.pkg` (lowercase)
4. **Rescan**: Click "Scan Directory" button
5. **Check logs**: Settings → Enable verbose logging

### Extraction Fails

**Common causes:**
- Insufficient disk space (need 2x PKG size)
- Write permissions to extraction path
- Corrupted PKG file
- Unsupported PKG type

**Solutions:**
```bash
# Check disk space
df -h /path/to/extraction

# Check permissions
ls -ld /path/to/extraction
chmod 755 /path/to/extraction

# Verify PKG integrity
md5sum file.pkg
```

### IGDB Not Working

**Check:**
1. Internet connection
2. API credentials correct
3. Credentials not expired
4. Rate limit not exceeded

**Reset:**
1. Settings → Clear IGDB credentials
2. Get new credentials from IGDB
3. Re-enter in Settings
4. Restart application

### License Activation Issues

**Error: "Invalid unlock code"**
- Code is case-sensitive
- Copy-paste instead of typing
- Verify no extra spaces
- Check System ID matches

**Error: "System ID mismatch"**
- Hardware may have changed
- Contact support with:
  - Old System ID (from registration)
  - New System ID (shown in app)
  - Proof of purchase

### Performance Issues

**Slow extraction:**
- Close other applications
- Extract to SSD instead of HDD
- Reduce concurrent operations

**High memory usage:**
- Expected for large PKGs
- Close unnecessary tabs
- Restart application periodically

**UI freezing:**
- Extraction runs in background
- Wait for completion
- Check Extraction Log for progress

## Advanced Usage

### Command-Line Arguments

```bash
# Verbose logging
./shadps4-manager-gui --verbose

# Custom config file
./shadps4-manager-gui --config=/path/to/config.ini

# Auto-start extraction
./shadps4-manager-gui --auto-extract

# Headless mode (future feature)
./shadps4-manager-gui --headless --extract=/path/to/file.pkg
```

### Environment Variables

```bash
# Override default paths
export SHADPS4_DOWNLOADS="/custom/downloads"
export SHADPS4_EXTRACTION="/custom/extraction"
export SHADPS4_INSTALLATION="/custom/games"

./shadps4-manager-gui
```

### Configuration File

**Location:**
- Linux: `~/.config/ShadPs4/ShadPs4 Manager.conf`
- Windows: `%APPDATA%\ShadPs4\ShadPs4 Manager.ini`
- macOS: `~/Library/Preferences/com.shadps4.ShadPs4Manager.plist`

**Backup:**
```bash
# Linux
cp ~/.config/ShadPs4/ShadPs4\ Manager.conf ~/backup.conf

# Restore
cp ~/backup.conf ~/.config/ShadPs4/ShadPs4\ Manager.conf
```

### Database Location

**Game metadata cache:**
- Linux: `~/.local/share/ShadPs4/`
- Windows: `%APPDATA%\ShadPs4\`
- macOS: `~/Library/Application Support/ShadPs4/`

### Logs

**Location:**
- Linux: `~/.local/share/ShadPs4/logs/`
- Windows: `%APPDATA%\ShadPs4\logs\`
- macOS: `~/Library/Application Support/ShadPs4/logs/`

**Enable verbose logging:**
Settings → Advanced → Enable verbose logging

## Building from Source

See [BUILD.md](BUILD.md) in this directory for detailed build instructions.

## Support

- **Issues**: [GitHub Issues](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- **Documentation**: [Main Documentation](../../README.md)
- **Registration**: See REGISTRATION_SYSTEM.md

## Related Documentation

- [Build Instructions](BUILD.md)
- [User Guide](USER_GUIDE.md)
- [Registration System](../../REGISTRATION_SYSTEM.md)
- [Main Project README](../../README.md)

---

**Version**: 1.0.1  
**License**: GPL-3.0  
**Website**: https://crownparkcomputing.github.io/ShadPs4Manager/
