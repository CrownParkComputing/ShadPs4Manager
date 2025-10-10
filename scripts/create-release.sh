#!/bin/bash
# Create optimized release builds for all platforms

set -e

VERSION=$(cat VERSION | tr -d '[:space:]')
BUILD_DIR="build-release"
RELEASE_DIR="release-v${VERSION}"

echo "====================================="
echo "  ShadPs4Manager Release Builder"
echo "  Version: ${VERSION}"
echo "====================================="

# Clean previous builds
echo "[1/5] Cleaning previous builds..."
rm -rf "$BUILD_DIR" "$RELEASE_DIR"
mkdir -p "$BUILD_DIR" "$RELEASE_DIR"

# Build with optimizations
echo "[2/5] Configuring CMake for release..."
cd "$BUILD_DIR"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -march=x86-64 -mtune=generic" \
    -DCMAKE_INSTALL_PREFIX="/usr/local"

echo "[3/5] Building all applications..."
make -j$(nproc)

echo "[4/5] Stripping binaries..."
strip bin/shadps4-manager-gui
strip bin/shadps4-pkg-extractor
strip bin/shadps4-unlock-code-generator

echo "[5/5] Creating release packages..."
cd ..

# Package for Linux
echo "  Creating Linux x64 package..."
mkdir -p "$RELEASE_DIR/linux-x64"
cp "$BUILD_DIR/bin/shadps4-manager-gui" "$RELEASE_DIR/linux-x64/"
cp "$BUILD_DIR/bin/shadps4-pkg-extractor" "$RELEASE_DIR/linux-x64/"
cp "$BUILD_DIR/bin/shadps4-unlock-code-generator" "$RELEASE_DIR/linux-x64/"
cp LICENSE "$RELEASE_DIR/linux-x64/"
cp README.md "$RELEASE_DIR/linux-x64/"

cat > "$RELEASE_DIR/linux-x64/INSTALL.txt" <<'EOF'
ShadPs4Manager Installation
============================

1. Make executables runnable:
   chmod +x shadps4-manager-gui
   chmod +x shadps4-pkg-extractor
   chmod +x shadps4-unlock-code-generator

2. Run the GUI:
   ./shadps4-manager-gui

3. System-wide installation (optional):
   sudo cp shadps4-* /usr/local/bin/

For detailed instructions, see README.md
EOF

# Create tarball
cd "$RELEASE_DIR"
tar -czf "shadps4-manager-linux-x64-v${VERSION}.tar.gz" linux-x64/
cd ..

# Show file sizes
echo ""
echo "====================================="
echo "  Build Complete!"
echo "====================================="
echo ""
ls -lh "$BUILD_DIR/bin/"
echo ""
echo "Release package:"
ls -lh "$RELEASE_DIR"/*.tar.gz
echo ""
echo "Release files ready in: $RELEASE_DIR/"
echo ""
echo "Next steps:"
echo "1. Test the applications"
echo "2. Create GitHub release"
echo "3. Upload: $RELEASE_DIR/shadps4-manager-linux-x64-v${VERSION}.tar.gz"
