#!/bin/bash
# Extract DRIVECLUB PKG with full progress reporting

PKG_FILE="/home/jon/Downloads/[DLPSGAME.COM]-DRIVECLUB.USA.PS4-CUSA00093.pkg"
OUTPUT_DIR="/home/jon/Downloads/DRIVECLUB_EXTRACTED"

echo "=================================="
echo "DRIVECLUB PKG Extraction"
echo "=================================="
echo ""
echo "PKG File: $PKG_FILE"
echo "Output:   $OUTPUT_DIR"
echo ""

# Check if PKG file exists
if [ ! -f "$PKG_FILE" ]; then
    echo "ERROR: PKG file not found: $PKG_FILE"
    exit 1
fi

# Show file size
PKG_SIZE=$(du -h "$PKG_FILE" | cut -f1)
echo "PKG Size: $PKG_SIZE"
echo ""

# Check available disk space
AVAIL=$(df -h "$(dirname "$OUTPUT_DIR")" | tail -1 | awk '{print $4}')
echo "Available Space: $AVAIL"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Run extraction with progress
echo "Starting extraction..."
echo ""

"$(dirname "$0")/build/bin/shadps4-pkg-extractor" "$PKG_FILE" "$OUTPUT_DIR"

EXIT_CODE=$?

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo "=================================="
    echo "Extraction completed successfully!"
    echo "=================================="
    echo ""
    echo "Files extracted to: $OUTPUT_DIR"
    
    # Show what was extracted
    echo ""
    echo "Extracted contents:"
    du -sh "$OUTPUT_DIR"
    echo ""
    ls -lh "$OUTPUT_DIR" | head -20
else
    echo "=================================="
    echo "Extraction FAILED with code $EXIT_CODE"
    echo "=================================="
fi

exit $EXIT_CODE
