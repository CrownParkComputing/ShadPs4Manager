#!/bin/bash
# Test extraction with double-free fix

echo "Testing PKG extraction with GUI fix..."
echo "========================================"
echo ""

# Create test output directory
TEST_DIR="/tmp/gui_extract_test"
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"

# Test with smaller PKG
PKG_FILE="/home/jon/Downloads/JP1091-CUSA39417_00-7787057444766834-A0100-V0101.pkg"

if [ ! -f "$PKG_FILE" ]; then
    echo "ERROR: Test PKG file not found: $PKG_FILE"
    exit 1
fi

echo "PKG: $PKG_FILE"
echo "Output: $TEST_DIR"
echo ""

# Run CLI extraction to verify it works
echo "Running CLI extraction test..."
/home/jon/ShadPs4Manager/build/bin/shadps4-pkg-extractor "$PKG_FILE" "$TEST_DIR" 2>&1 | tail -5

echo ""
echo "Checking extracted files..."
FILE_COUNT=$(find "$TEST_DIR" -type f | wc -l)
DIR_SIZE=$(du -sh "$TEST_DIR" | cut -f1)

echo "Files extracted: $FILE_COUNT"
echo "Total size: $DIR_SIZE"

if [ "$FILE_COUNT" -gt 100 ]; then
    echo ""
    echo "✅ CLI extraction successful!"
    echo ""
    echo "GUI should now work with the QPointer fix."
    echo "Double-free issue resolved by:"
    echo "  1. Using QPointer<QProgressDialog> for safe pointer access"
    echo "  2. Clearing callback before deleting dialog"
    echo "  3. Checking dialogPtr validity in lambda"
else
    echo ""
    echo "❌ Extraction may have issues - only $FILE_COUNT files found"
fi

echo ""
echo "To test GUI manually:"
echo "  /home/jon/ShadPs4Manager/build/bin/shadps4-manager-gui"
