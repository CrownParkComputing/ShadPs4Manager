# ShadPs4 Manager - Registration System Implementation

## Overview
The ShadPs4 Manager now includes a full registration/licensing system with trial limitations and PayPal integration.

## Features Implemented

### 1. Splash Screen Removed ✅
- Removed the splash screen on application startup for faster launch
- Users go directly to the main interface

### 2. Welcome/Registration Tab ✅
The application now shows a **Welcome tab** for unregistered users containing:

#### Step 1: System ID Display
- Generates a unique System ID based on hardware (CPU, MAC addresses, hostname)
- System ID is a SHA-256 hash ensuring uniqueness per machine
- Copy button to easily copy the System ID to clipboard

#### Step 2: PayPal Payment Integration
- Embedded PayPal hosted button for license purchase
- Instructions to include System ID in payment notes
- Customer receives unlock code via email to their PayPal address

#### Step 3: License Activation
- Input field for entering the unlock code received via email
- Validation against System ID + secret word
- Automatic application restart after successful activation

### 3. License Validation System ✅
**Secret Word**: `AstroCity_EGRET_II`

**Algorithm**:
```
Unlock Code = SHA256(System ID + "AstroCity_EGRET_II")
```

**Storage**:
- Registration status stored in QSettings
- License key stored encrypted
- System ID stored for validation

### 4. 10-Item Trial Limitation ✅
**Unregistered users are limited to:**
- Maximum 10 total installed items
- Count includes: Base Games + Updates + DLC
- Warning dialog shown when limit would be exceeded
- Clear message directing users to registration tab

**Enforcement**:
- Check performed before every installation
- Counts existing installed items
- Calculates items to be installed
- Blocks installation if total > 10

### 5. Unlock Code Generator (Admin Tool) ✅
**Separate application**: `shadps4-unlock-code-generator`

**Features**:
- Simple Qt GUI for administrators
- Input: Customer's System ID (from their Welcome tab)
- Output: Generated unlock code
- Copy to clipboard functionality
- Generation history log with timestamps
- Validates System ID format (64-character hex)

**Usage**:
1. Customer makes PayPal payment and sends their System ID
2. Admin pastes System ID into generator
3. Generator creates unlock code using same algorithm
4. Admin emails unlock code to customer
5. Customer enters code in Welcome tab to activate

## Files Modified

### Core Application Files:
- **src/gui/working_gui.cpp** - Removed splash, added welcome tab integration
- **src/gui/settings.h/cpp** - Added license management methods
- **src/gui/downloads_folder.cpp** - Added 10-item limit check
- **src/gui/installation_folder.h/cpp** - Added installed item counting

### New Files Created:
- **src/gui/welcome_tab.h** - Welcome/Registration tab header
- **src/gui/welcome_tab.cpp** - Welcome/Registration tab implementation  
- **src/gui/unlock_code_generator.cpp** - Admin unlock code generator

### Build System:
- **CMakeLists.txt** - Added welcome_tab.cpp and unlock_code_generator executable

## Build Outputs

### Main Application:
```bash
./build/bin/shadps4-manager-gui
```

### Admin Tool:
```bash
./build/bin/shadps4-unlock-code-generator
```

## PayPal Integration Details

**Client ID**: `BAAgevXb9mhpMdSYaxkZJbJ2XOvoewp6TLuo_kj1d14YxzUdCcJKIhqVPSGleAt1n2euXdbMn4N8DF2zsE`
**Button ID**: `2WY8TPHV3KDMS`
**Currency**: GBP

The PayPal button is embedded in the Welcome tab using PayPal's Hosted Buttons SDK.

## User Flow

### For New Users (Unregistered):
1. Launch ShadPs4 Manager
2. See Welcome tab as first tab (with ⚠️ icon)
3. Copy their unique System ID
4. Click PayPal button and complete payment (including System ID in notes)
5. Receive unlock code via email
6. Enter unlock code in Welcome tab
7. Application validates and restarts
8. Welcome tab disappears, full access granted

### For Registered Users:
1. Launch ShadPs4 Manager
2. No Welcome tab shown
3. All features unlocked
4. Unlimited installations

## Technical Details

### System ID Generation:
- Uses QSysInfo::machineUniqueId()
- Includes hostname
- Includes all non-zero MAC addresses
- Includes OS product type and version
- SHA-256 hash of combined identifiers
- Result: 64-character hex string

### License Validation:
- Stored in QSettings under "license/" keys
- `license/is_registered` - Boolean registration status
- `license/key` - The unlock code
- `license/system_id` - The system's unique ID

### Security:
- Unlock codes are cryptographically bound to specific hardware
- Cannot be shared between machines
- SHA-256 ensures collision resistance
- Secret word adds additional entropy

## Testing

### Test Unregistered State:
```bash
# Clear registration (for testing)
rm ~/.config/ShadPs4/ShadPs4\ Manager.conf
./build/bin/shadps4-manager-gui
```

### Generate Test Unlock Code:
```bash
# Run admin tool
./build/bin/shadps4-unlock-code-generator

# Or manually calculate for a System ID:
echo -n "<SYSTEM_ID>AstroCity_EGRET_II" | sha256sum
```

## Future Enhancements (Optional)

- Online license validation server
- License transfer mechanism
- Subscription-based licensing
- Hardware change detection and grace period
- Multi-machine licenses for businesses

## Support

For license issues, customers should provide:
- Their System ID
- PayPal transaction ID
- Email address used for PayPal

Admin can then regenerate the unlock code if needed.
