# Unlock Code Generator - Record Keeping System

## New Features

### 1. Email Address Recording
The generator now requires and stores customer email addresses along with System IDs.

### 2. Encrypted Storage
All records are stored in an encrypted JSON file with the following security:
- **Email addresses**: Encrypted using XOR cipher with Base64 encoding
- **Encryption key**: `AstroCity_EGRET_II_AdminKey`
- **Storage location**: System's application data directory
  - Linux: `~/.local/share/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`
  - Windows: `%APPDATA%/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`
  - macOS: `~/Library/Application Support/ShadPs4 Manager Unlock Code Generator/unlock_code_records.json`

### 3. Persistent Record History
- All generated unlock codes are automatically saved
- Records persist across application restarts
- History log shows all previous generations in the current session

### 4. View All Records
New "📋 View All Records" button displays:
- Complete history of all generated codes
- Decrypted email addresses
- System IDs
- Unlock codes
- Timestamps

## Updated Usage Flow

### Step 1: Collect Customer Information
After PayPal payment, collect:
- Customer's **System ID** (64-character hex)
- Customer's **Email Address** (for record keeping and verification)

### Step 2: Generate Code
1. Paste System ID in first field
2. Enter customer's email in second field
3. Click "🔑 Generate Unlock Code"
4. Code appears in output box

### Step 3: Record Keeping
The system automatically:
- Encrypts the email address
- Saves record to encrypted JSON file
- Adds to session history log
- Timestamp is recorded

### Step 4: View Records
Click "📋 View All Records" to:
- See all historical generations
- Verify previous customers
- Look up codes by email or System ID
- Audit trail for all license generations

## Security Features

### Encryption Algorithm
```
Encrypted = Base64(XOR(PlainText, Key))
Decrypted = XOR(Base64Decode(Encrypted), Key)
```

### What's Encrypted
- ✅ Email addresses (fully encrypted)
- ❌ System IDs (not encrypted - hardware hashes, not sensitive)
- ❌ Unlock codes (not encrypted - public keys once sent)
- ❌ Timestamps (not encrypted - not sensitive)

### Why XOR + Base64?
- Simple but effective for local storage
- Prevents casual viewing of email addresses
- Easy to decrypt when needed (admin tool only)
- No external dependencies
- Fast performance

## Record Format

### JSON Structure (on disk)
```json
[
  {
    "timestamp": "2025-10-10 14:30:45",
    "systemId": "ABC123...",
    "encryptedEmail": "SGVsbG8gV29ybGQh...",
    "unlockCode": "DEF456..."
  }
]
```

### Decrypted View (in application)
```
=== Record ===
Date: 2025-10-10 14:30:45
System ID: ABC123...
Email: customer@example.com
Unlock Code: DEF456...
```

## Benefits

### For Admin
1. **Audit Trail**: Complete history of all license generations
2. **Customer Support**: Quickly look up customer records
3. **Verification**: Confirm previous generations before regenerating
4. **Record Keeping**: Automatic, no manual tracking needed

### For Business
1. **Compliance**: Records of all license sales
2. **Support**: Easy customer verification
3. **Tracking**: Who bought what and when
4. **Security**: Email addresses protected at rest

## Backup & Recovery

### Backup Records
Simply copy the JSON file:
```bash
# Linux
cp ~/.local/share/ShadPs4\ Manager\ Unlock\ Code\ Generator/unlock_code_records.json backup.json

# Windows
copy "%APPDATA%\ShadPs4 Manager Unlock Code Generator\unlock_code_records.json" backup.json
```

### Restore Records
Copy the file back to the application data directory.

### Transfer Between Machines
1. Export the JSON file from machine A
2. Copy to machine B
3. Place in the application data directory
4. Launch the generator - records will load automatically

## Privacy & Legal

### Data Handling
- Email addresses are encrypted at rest
- Records stored locally only (not transmitted)
- No cloud storage or external services
- Admin has full control of data

### GDPR Considerations
- Customer data (email) is encrypted
- Records can be deleted by removing entries from JSON
- No automated profiling or processing
- Data used only for license verification

### Retention
- Records persist indefinitely by default
- Can be manually cleaned by editing/deleting JSON file
- Consider implementing retention policy as needed

## Troubleshooting

### Records Not Showing
- Check file permissions on JSON file
- Verify file exists in correct location
- Try regenerating - should create new file

### Decryption Fails
- Don't modify the encryption key in code
- Ensure JSON file not corrupted
- Backup and regenerate if necessary

### Lost Records
- Restore from backup JSON file
- Records cannot be recovered if file is lost
- Regular backups recommended

## Technical Details

### Dependencies
- Qt Core (QFile, QDir, QStandardPaths)
- Qt JSON (QJsonDocument, QJsonObject, QJsonArray)
- No external crypto libraries needed

### Performance
- Instant encryption/decryption
- JSON parsing is fast even with 1000+ records
- No database overhead
- File size: ~200 bytes per record

### Limitations
- File-based storage (not suitable for millions of records)
- Simple XOR encryption (not military-grade)
- Single file (no sharding or distribution)
- Manual backup required

## Future Enhancements (Optional)

Potential improvements:
- [ ] AES encryption for email addresses
- [ ] SQLite database for large record sets
- [ ] Search/filter functionality
- [ ] Export to CSV for accounting
- [ ] Automatic backups
- [ ] Multiple admin users with access control
- [ ] Cloud sync option
