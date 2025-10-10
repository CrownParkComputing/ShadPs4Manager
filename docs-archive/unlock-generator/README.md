# Unlock Code Generator

Admin tool for generating license unlock codes for ShadPs4 Manager.

## Overview

The Unlock Code Generator is a standalone Qt6 application used by administrators to generate license unlock codes for registered users. It includes:
- System ID-based code generation
- Encrypted customer email storage
- Generation history tracking
- Persistent record keeping

**Size**: ~85 KB  
**License**: GPL-3.0  
**Audience**: Administrators only

## ⚠️ Important Notice

This tool is **ONLY** for administrators who need to generate unlock codes for customers. Regular users should:
1. Use the main Manager GUI application
2. Register via PayPal
3. Receive unlock code from admin
4. Enter code in the application

**Do not distribute this tool to end users.**

## Features

### Code Generation
- Generate unlock codes from System IDs
- SHA-256-based secure algorithm
- Secret key: "AstroCity_EGRET_II"
- Codes are 64-character hex strings

### Customer Records
- Store customer email addresses (encrypted)
- XOR cipher with Base64 encoding
- Encryption key: "AstroCity_EGRET_II_AdminKey"
- JSON-based persistent storage

### History Tracking
- Session history of all generated codes
- View all records from previous sessions
- Search and filter functionality
- Export capabilities

### User Interface
- **Generate Tab**: Create new unlock codes
- **History Tab**: View past generations
- Compact design (fits small screens)
- Qt6 native interface

## System Requirements

### Minimum Requirements
- **OS**: Linux, Windows, macOS
- **CPU**: Any modern CPU
- **RAM**: 128 MB
- **Disk**: 1 MB
- **Display**: 700x600 minimum

### Dependencies
- Qt6 (Widgets only)
- No other dependencies
- No network required

## Installation

### Linux

```bash
# Extract archive
tar -xzf shadps4-unlock-code-generator-linux-x64-v1.0.1.tar.gz

# Make executable
chmod +x shadps4-unlock-code-generator

# Run
./shadps4-unlock-code-generator
```

### Windows

1. Extract ZIP file
2. Double-click `shadps4-unlock-code-generator.exe`

### macOS

```bash
# Extract archive
tar -xzf shadps4-unlock-code-generator-macos-universal-v1.0.1.tar.gz

# Make executable
chmod +x shadps4-unlock-code-generator

# Run
./shadps4-unlock-code-generator
```

## Usage

### Generating an Unlock Code

#### Step 1: Get Customer System ID

Customer provides their System ID from the Manager GUI Welcome tab.

**Example System ID:**
```
a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6
```

#### Step 2: Open Generator

Launch the Unlock Code Generator application.

#### Step 3: Generate Tab

1. **System ID field**: Paste customer's System ID
2. **Email field**: Enter customer's email address
3. Click **"Generate Unlock Code"**
4. Unlock code appears in output field
5. **Copy to clipboard** button copies code
6. Record added to history automatically

#### Step 4: Send Code to Customer

Send the unlock code via:
- Email
- Support ticket system
- Secure messaging
- Any secure channel

Customer enters code in Manager GUI → Welcome Tab → License Activation.

### Viewing History

#### Session History

Shows codes generated in current session:
- System ID (truncated)
- Email address
- Unlock code (truncated)
- Timestamp

#### All Records

Click **"View All Records"** to see persistent history:
- All previous sessions
- Searchable
- Sortable by date
- Export to CSV (future feature)

### Record Management

Records are automatically saved to:
- **Linux**: `~/.local/share/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`
- **Windows**: `%APPDATA%\ShadPs4 Manager Unlock Code Generator\unlock_code_records.json`
- **macOS**: `~/Library/Application Support/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`

## Record Format

### JSON Structure

```json
{
    "version": "1.0",
    "records": [
        {
            "systemId": "a1b2c3d4...",
            "email": "BASE64_ENCRYPTED_EMAIL",
            "unlockCode": "0123456789abcdef...",
            "timestamp": "2025-10-10T12:34:56"
        }
    ]
}
```

### Email Encryption

Emails are encrypted using:
1. **XOR cipher** with key "AstroCity_EGRET_II_AdminKey"
2. **Base64 encoding** for storage
3. Decrypted on-demand when viewing

**Example:**
```
Original: customer@example.com
Encrypted: QWFhYWFhYWFhYWFhYQ==
```

## Security

### Unlock Code Algorithm

```cpp
QString unlockCode = QCryptographicHash::hash(
    (systemId + "AstroCity_EGRET_II").toUtf8(),
    QCryptographicHash::Sha256
).toHex();
```

**Key Features:**
- SHA-256 hashing
- Deterministic (same System ID → same code)
- Secret word adds security
- 64-character hex output

### Email Encryption

```cpp
QByteArray encrypted;
QString key = "AstroCity_EGRET_II_AdminKey";
for (int i = 0; i < email.length(); i++) {
    encrypted.append(email[i].toLatin1() ^ key[i % key.length()].toLatin1());
}
QString stored = encrypted.toBase64();
```

**Properties:**
- Simple XOR cipher (adequate for email storage)
- Base64 encoding for JSON compatibility
- Reversible (for viewing history)
- Not suitable for highly sensitive data

### Threat Model

**Protects against:**
- Casual viewing of email addresses in JSON file
- Accidental exposure in backups
- Plain-text data leaks

**Does NOT protect against:**
- Determined attacker with JSON file access
- Cryptographic attacks (use stronger encryption if needed)
- Physical access to running application

**Recommendation**: Store JSON file on encrypted disk.

## Administration Tasks

### Adding New Admin User

1. Copy executable to admin's machine
2. No shared records (each admin has separate history)
3. Optionally share records file if centralized tracking needed

### Centralized Record Keeping

**Option 1: Shared Network Drive**
```bash
# Linux: Symlink to network share
ln -s /mnt/shared/unlock_records.json \
  ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json
```

**Option 2: Database Integration**
Modify source code to store in MySQL/PostgreSQL instead of JSON.

**Option 3: Cloud Sync**
Store records directory in Dropbox/Google Drive.

### Backup Procedures

**Manual Backup:**
```bash
# Linux
cp ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json \
   ~/backup-$(date +%Y%m%d).json

# Windows (PowerShell)
Copy-Item "$env:APPDATA\ShadPs4 Manager Unlock Code Generator\unlock_code_records.json" `
  "backup-$(Get-Date -Format 'yyyyMMdd').json"
```

**Automated Backup (Linux):**
```bash
# Cron job: daily at 3 AM
0 3 * * * cp ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json \
  ~/backups/unlock_records_$(date +\%Y\%m\%d).json
```

### Recovery

**Restore from backup:**
```bash
# Linux
cp ~/backup-20251010.json \
  ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json

# Restart application
```

### Audit Trail

Records include timestamps. To audit:

```bash
# View all records (Linux)
cat ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json | jq

# Count records
jq '.records | length' unlock_code_records.json

# Find specific email (requires decryption)
# Use "View All Records" in application
```

## Troubleshooting

### Application Won't Start

**Linux:**
```bash
# Check Qt6 dependencies
ldd shadps4-unlock-code-generator

# Install Qt6 if missing
sudo apt install qt6-base-dev
```

**Windows:**
- Install Visual C++ Redistributables
- Ensure Qt6 DLLs are present

### Records Not Saving

1. Check write permissions:
```bash
# Linux
ls -ld ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/

# Fix permissions
chmod 755 ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/
```

2. Check disk space:
```bash
df -h ~
```

3. Verify JSON file:
```bash
cat unlock_code_records.json | jq
# If invalid, restore from backup
```

### Invalid Unlock Code Generated

**Problem**: Code doesn't work in Manager GUI

**Causes:**
1. System ID was incorrect (customer typo)
2. System ID changed (hardware change)
3. Customer using different machine

**Solution:**
1. Ask customer to copy System ID again
2. Regenerate code with correct System ID
3. Verify customer hasn't changed hardware

### History Not Showing

1. Check file exists:
```bash
ls ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json
```

2. Check file is valid JSON:
```bash
cat unlock_code_records.json | jq
```

3. Restore from backup if corrupted

## Advanced Usage

### Command-Line Generation (Future Feature)

```bash
# Not yet implemented
shadps4-unlock-code-generator --system-id "abc123..." --email "user@example.com"
```

### Bulk Generation

For multiple codes, use UI repeatedly or modify source for batch mode.

### Custom Encryption

To use stronger encryption, modify source code:

```cpp
// Replace XOR cipher with AES-256
#include <QAesEncryption>

QString encryptEmail(const QString& email) {
    QAesEncryption encryption(QAesEncryption::AES_256, 
                              QAesEncryption::CBC);
    QByteArray key = "your-32-byte-key-here...";
    QByteArray iv = QAesEncryption::Rng::generate(16);
    QByteArray encrypted = encryption.encode(email.toUtf8(), key, iv);
    return QString(encrypted.toBase64());
}
```

## Building from Source

See [BUILD.md](BUILD.md) for build instructions.

## Security Best Practices

1. **Limit Access**: Only trusted admins should have this tool
2. **Secure Storage**: Keep executable on encrypted drive
3. **Backup Records**: Regular automated backups
4. **Audit Regularly**: Review generation history
5. **Secure Communication**: Send codes via encrypted channels
6. **Verify Identity**: Confirm customer identity before generating
7. **Revocation**: No built-in revocation (future feature)

## Workflow Example

### Standard Registration Flow

1. **Customer**: Purchases via PayPal
2. **Customer**: Emails System ID to support
3. **Admin**: Verifies payment in PayPal
4. **Admin**: Opens Unlock Code Generator
5. **Admin**: Enters System ID and customer email
6. **Admin**: Clicks "Generate Unlock Code"
7. **Admin**: Copies unlock code
8. **Admin**: Emails code to customer
9. **Customer**: Enters code in Manager GUI
10. **Customer**: Full access unlocked

### Bulk Registration (Event/Promotion)

1. Collect System IDs and emails in spreadsheet
2. Open Unlock Code Generator
3. Generate codes one by one (or implement batch mode)
4. Export codes to spreadsheet
5. Send bulk emails with codes

## Related Documentation

- [Admin Guide](../../ADMIN_GUIDE.md) - General admin procedures
- [Record Keeping](../../RECORD_KEEPING.md) - Detailed record documentation
- [Registration System](../../REGISTRATION_SYSTEM.md) - Technical details

## Support

- **Issues**: [GitHub Issues](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- **Security**: Email security concerns privately
- **Build Guide**: [BUILD.md](BUILD.md)

---

**Version**: 1.0.1  
**License**: GPL-3.0  
**Audience**: Administrators Only  
**Website**: https://crownparkcomputing.github.io/ShadPs4Manager/
