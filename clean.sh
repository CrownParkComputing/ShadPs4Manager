#!/bin/bash
# Clean build artifacts and temporary files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo "Cleaning build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    echo "  Removing build/"
    rm -rf build
fi

# Remove CMake cache files
find . -name "CMakeCache.txt" -delete
find . -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "cmake_install.cmake" -delete
find . -name "Makefile" -type f -not -path "./external/*" -delete

# Remove autogen files
find . -name "*_autogen" -type d -exec rm -rf {} + 2>/dev/null || true

# Remove Qt moc/ui generated files
find . -name "moc_*.cpp" -delete
find . -name "ui_*.h" -delete

# Remove object files and libraries
find . -name "*.o" -delete
find . -name "*.a" -delete
find . -name "*.so" -delete
find . -name "*.dylib" -delete

# Remove backup files
find . -name "*~" -delete
find . -name "*.bak" -delete

echo "Clean complete!"
echo ""
echo "To rebuild, run:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j\$(nproc)"
