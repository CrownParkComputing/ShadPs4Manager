# Unlock Code Generator - Admin Guide

## Purpose
This tool generates unlock codes for customers who have purchased ShadPs4 Manager licenses through PayPal.

## Location
```bash
./build/bin/shadps4-unlock-code-generator
```

## How to Use

### Step 1: Receive Customer Information
After a customer completes PayPal payment, they should email you:
- Their **System ID** (64-character hex string from the Welcome tab)
- Their **PayPal transaction ID** (for verification)
- The **email address** used for PayPal payment

### Step 2: Launch Generator
```bash
cd /home/jon/ShadPs4Manager/build/bin
./shadps4-unlock-code-generator
```

### Step 3: Generate Code
1. Copy the customer's System ID from their email
2. Paste it into the "Customer's System ID" field
3. Click "🔑 Generate Unlock Code"
4. The unlock code will appear in the output box

### Step 4: Send to Customer
1. Click "📋 Copy Unlock Code"
2. Email the code to the customer's PayPal address
3. Include activation instructions

### Email Template
```
Subject: Your ShadPs4 Manager Unlock Code

Dear Customer,

Thank you for purchasing ShadPs4 Manager!

Your unlock code is:
[PASTE_CODE_HERE]

To activate your license:
1. Open ShadPs4 Manager
2. Go to the Welcome tab
3. Enter the unlock code in the activation field
4. Click "Activate License"
5. The application will restart with full features unlocked

If you have any issues, please reply to this email with:
- Your System ID
- The error message you're seeing

Thank you for your purchase!
```

## Validation

The tool will warn you if:
- System ID field is empty
- System ID is not 64 characters long
- System ID format appears invalid

## History Log

The tool keeps a session history of all generated codes:
- System ID
- Generated unlock code
- Timestamp

This helps track multiple generations in one session.

## Security Notes

### Important:
- **Each System ID generates a UNIQUE unlock code**
- Codes are hardware-locked and cannot be shared
- The secret word `AstroCity_EGRET_II` is embedded in the algorithm
- Uses SHA-256 cryptographic hashing

### Do NOT:
- Share unlock codes publicly
- Give one customer's code to another customer
- Modify the secret word (will break all codes)

## Troubleshooting

### Customer Says Code Doesn't Work

**Checklist**:
1. Verify they copied their System ID correctly (exactly 64 characters)
2. Regenerate the code with their System ID
3. Check for extra spaces or characters when they paste
4. Ensure they're entering the code in the main app, not generator

**Common Issues**:
- Customer sent wrong System ID (maybe from different machine)
- Copy/paste error (missing characters)
- Typing error when entering code
- Using code on different computer

**Solution**:
Ask customer to:
1. Send a screenshot of their Welcome tab showing System ID
2. Verify the System ID matches what they sent
3. Try regenerating and resending the code

### Code Won't Generate

**Check**:
- System ID is exactly 64 hexadecimal characters
- No spaces before/after the System ID
- Using the current version of the generator

## Algorithm Details

For reference, the unlock code generation uses:
```
Unlock Code = SHA256(SystemID + "AstroCity_EGRET_II")
```

Both parts must be in UPPERCASE for consistency.

## Building from Source

If you need to rebuild the generator:
```bash
cd /home/jon/ShadPs4Manager/build
cmake ..
make shadps4-unlock-code-generator
```

The source is in: `src/gui/unlock_code_generator.cpp`

## Contact

For issues with the generator tool itself, contact the development team.
For customer license issues, use the email template above.
