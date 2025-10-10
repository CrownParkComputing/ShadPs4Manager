# ShadPs4Manager - Deployment & Testing Guide

## 🚀 Deployment Status

### Version
- **Current Version**: 1.0.1
- **Build Status**: ✅ All three applications build successfully

### GitHub Actions Workflow
- **Location**: `.github/workflows/release.yml`
- **Status**: ✅ Configured and ready
- **Platforms**: Linux x64, Windows x64
- **Trigger**: Manual workflow dispatch or git tag push (`v*`)

### Release Process

#### Option 1: Automated GitHub Release (Recommended)
```bash
# Go to GitHub Actions tab
# Click "Release" workflow
# Click "Run workflow"
# Select version bump: patch/minor/major
# Enable "Create GitHub release"
# Click "Run workflow" button

# Workflow will:
# 1. Bump version in CMakeLists.txt
# 2. Create git tag (e.g., v1.0.2)
# 3. Build Linux x64 binaries
# 4. Build Windows x64 binaries
# 5. Create release packages
# 6. Publish GitHub release with assets
```

#### Option 2: Manual Local Build
```bash
# Run the release build script
./scripts/create-release.sh

# This creates:
# - build-release/ directory with optimized binaries
# - shadps4-manager-linux-x64-v1.0.1.tar.gz package

# Upload manually to GitHub:
# 1. Go to Releases → Draft a new release
# 2. Create tag: v1.0.1
# 3. Upload the .tar.gz file
# 4. Publish release
```

---

## 💳 Payment Integration Testing

### PayPal Configuration
- **Payment URL**: https://www.paypal.com/ncp/payment/2WY8TPHV3KDMS
- **Price**: $9.99 USD
- **Payment Type**: One-time purchase
- **Integration**: Qt QDesktopServices (external browser)

### Testing Checklist

#### ✅ PayPal Button Test
1. **Launch GUI**:
   ```bash
   cd /home/jon/ShadPs4Manager/build/bin
   ./shadps4-manager-gui
   ```

2. **Verify Welcome Tab**:
   - [ ] System ID is displayed at the top
   - [ ] System ID is unique (64-character hex string)
   - [ ] "Copy System ID" button works
   - [ ] System ID copies to clipboard

3. **Test PayPal Button**:
   - [ ] "💳 Pay with PayPal" button is visible
   - [ ] Button click opens browser
   - [ ] Correct URL opens: https://www.paypal.com/ncp/payment/2WY8TPHV3KDMS
   - [ ] PayPal page loads correctly
   - [ ] Payment form displays $9.99 USD

4. **Test Information Dialog**:
   - [ ] Message box appears after button click
   - [ ] Contains System ID
   - [ ] Instructions are clear
   - [ ] Close button works

#### ✅ License Activation Test
1. **Generate Test License**:
   ```bash
   # Get System ID from GUI (shown in Welcome tab)
   SYSTEM_ID="<your-system-id>"
   
   # Generate unlock code
   cd /home/jon/ShadPs4Manager/build/bin
   ./shadps4-unlock-code-generator --generate "$SYSTEM_ID"
   ```

2. **Test Activation**:
   - [ ] Paste unlock code into "Enter unlock code" field
   - [ ] Click "🔓 Activate License" button
   - [ ] Success message appears
   - [ ] License status changes to "Full Version"
   - [ ] 10-item limit is removed
   - [ ] License persists after restart

#### ✅ Trial Mode Test
1. **Fresh Install Behavior**:
   - [ ] GUI starts in trial mode
   - [ ] Trial counter shows "0/10 items"
   - [ ] Warning appears at 8/10 items
   - [ ] Blocking message at 10/10 items

2. **Registration Prompts**:
   - [ ] Clear instructions to upgrade
   - [ ] PayPal button easily accessible
   - [ ] System ID clearly displayed

---

## 📝 Payment Workflow Documentation

### Customer Journey
```
1. Download & Install
   ↓
2. Trial Mode (10 games)
   ↓
3. Trial Limit Reached
   ↓
4. Click PayPal Button in Welcome Tab
   ↓
5. Complete Payment ($9.99 USD)
   ↓
6. Email System ID to: support@crownparkcomputing.com
   ↓
7. Receive Unlock Code (within 24 hours)
   ↓
8. Enter Code in GUI
   ↓
9. Full Version Activated ✅
```

### System ID Details
- **Generation**: SHA-256 hash of CPU ID + Motherboard Serial + Hostname
- **Format**: 64-character hexadecimal string
- **Purpose**: Hardware-locked license (prevents sharing)
- **Location**: Displayed in Welcome tab of GUI
- **Persistence**: Stored in `~/.config/ShadPs4Manager/config.ini`

### Unlock Code Details
- **Generation**: `SHA-256(System ID + "AstroCity_EGRET_II")`
- **Format**: 64-character hexadecimal string
- **Validation**: Must match hardware-derived hash
- **Admin Tool**: `shadps4-unlock-code-generator` (encrypted database)
- **Security**: Only admins can generate codes

### Support Email Template
```
Subject: ShadPs4Manager License Request

Hello,

I've completed payment via PayPal and would like to activate my license.

System ID: [paste from GUI]
PayPal Transaction ID: [optional]
PayPal Email: [your email]

Thank you!
```

---

## 🔧 Testing Commands

### Build All Applications
```bash
cd /home/jon/ShadPs4Manager
./scripts/build.sh
```

### Test GUI Launch
```bash
cd build/bin
./shadps4-manager-gui &
```

### Test PKG Extractor
```bash
cd build/bin
./shadps4-pkg-extractor --help
./shadps4-pkg-extractor sample.pkg output/
```

### Test Unlock Generator
```bash
cd build/bin
./shadps4-unlock-code-generator --help
./shadps4-unlock-code-generator --generate "<system-id>"
```

### Check File Sizes
```bash
ls -lh build/bin/
# shadps4-manager-gui: ~3.8 MB
# shadps4-pkg-extractor: ~3.0 MB
# shadps4-unlock-code-generator: ~85 KB
```

---

## 📦 Release Package Contents

### Linux x64 Package (Public Release)
```
shadps4-manager-linux-x64-v1.0.1.tar.gz
├── shadps4-manager-gui           (3.6 MB)
├── shadps4-pkg-extractor         (3.0 MB)
├── LICENSE
├── README.md
└── INSTALL.txt
```

**Note**: unlock-code-generator is NOT included (admin-only tool, kept private)

### Windows x64 Package (Public Release)
```
ShadPs4Manager-1.0.1-windows-x64.zip
├── shadps4-manager-gui.exe
├── shadps4-pkg-extractor.exe
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── [other Qt DLLs]
├── LICENSE
└── README.md
```

**Note**: unlock-code-generator is NOT included (admin-only tool, kept private)

---

## ✅ Pre-Release Checklist

### Documentation
- [x] README.md updated with payment details
- [x] PayPal URL documented
- [x] Support email provided
- [x] Trial limits explained
- [x] License activation steps clear

### Code
- [x] All applications build without errors
- [x] PayPal URL hardcoded correctly
- [x] System ID generation works
- [x] License validation functional
- [x] Trial counter enforced

### Testing
- [ ] PayPal button opens correct URL
- [ ] Payment page displays $9.99 USD
- [ ] License activation works end-to-end
- [ ] Trial mode limits enforced
- [ ] GUI stable on fresh system

### Deployment
- [x] GitHub Actions workflow configured
- [x] Release script created (scripts/create-release.sh)
- [x] Version set to 1.0.1
- [ ] Git repository pushed to GitHub
- [ ] First release created

---

## 🚨 Known Issues / Notes

### PayPal Testing
- **Important**: Clicking the PayPal button will open the LIVE payment page
- Test carefully - real payments will be processed
- Use PayPal sandbox for development testing
- Current URL is production-ready

### License System
- System ID changes if hardware changes (CPU/motherboard)
- Unlock codes are hardware-locked
- No automated license delivery (manual via email)
- Admin must run unlock generator tool

### Build System
- Qt6 6.2+ required for GUI
- CMake 3.20+ required
- Linux builds tested on Ubuntu 22.04
- Windows builds tested on Windows 10/11

---

## 📧 Support Contact

**Email**: support@crownparkcomputing.com
**Response Time**: Within 24 hours
**Include**: System ID, PayPal transaction ID

---

## 🔗 Links

- **GitHub Repository**: https://github.com/CrownParkComputing/ShadPs4Manager
- **Website**: https://crownparkcomputing.github.io/ShadPs4Manager/
- **PayPal Payment**: https://www.paypal.com/ncp/payment/2WY8TPHV3KDMS
- **ShadPS4 Emulator**: https://github.com/shadps4-emu/shadPS4

---

*Last Updated: 2025-01-XX*
*Version: 1.0.1*
