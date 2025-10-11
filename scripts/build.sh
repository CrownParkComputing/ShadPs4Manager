#!/bin/bash

# ShadPs4Manager Build Script
# Usage: ./build.sh [major|minor|patch|clean] [linux|windows|all]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="ShadPs4Manager"
BUILD_DIR="build"
DIST_DIR="dist"
CMAKE_MIN_VERSION="3.16"

# Version file location
VERSION_FILE="VERSION"
CMAKE_VERSION_FILE="CMakeLists.txt"

# Print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get current version from CMakeLists.txt
get_current_version() {
    if [ -f "$CMAKE_VERSION_FILE" ]; then
        grep "project.*VERSION" "$CMAKE_VERSION_FILE" | sed -n 's/.*VERSION \([0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/p'
    else
        echo "1.0.0"
    fi
}

# Increment version based on type
increment_version() {
    local version=$1
    local type=$2
    
    IFS='.' read -ra VERSION_PARTS <<< "$version"
    local major=${VERSION_PARTS[0]}
    local minor=${VERSION_PARTS[1]}
    local patch=${VERSION_PARTS[2]}
    
    case $type in
        "major")
            major=$((major + 1))
            minor=0
            patch=0
            ;;
        "minor")
            minor=$((minor + 1))
            patch=0
            ;;
        "patch")
            patch=$((patch + 1))
            ;;
        *)
            print_error "Invalid version increment type: $type"
            exit 1
            ;;
    esac
    
    echo "$major.$minor.$patch"
}

# Update version in CMakeLists.txt
update_cmake_version() {
    local new_version=$1
    print_info "Updating CMakeLists.txt version to $new_version"
    
    sed -i "s/project(ShadPs4Manager VERSION [0-9]\+\.[0-9]\+\.[0-9]\+)/project(ShadPs4Manager VERSION $new_version)/" "$CMAKE_VERSION_FILE"
    
    # Also update the version file template
    if [ -f "src/common/scm_rev.cpp.in" ]; then
        sed -i "s/constexpr char g_version\\[\\]  = \"[0-9]\+\.[0-9]\+\.[0-9]\+\";/constexpr char g_version[]  = \"$new_version\";/" "src/common/scm_rev.cpp.in"
    fi
    
    # Save version to VERSION file
    echo "$new_version" > "$VERSION_FILE"
}

# Get git information
get_git_info() {
    if command -v git &> /dev/null && [ -d ".git" ]; then
        GIT_REV=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
        GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
        GIT_DESC=$(git describe --always --dirty --tags 2>/dev/null || echo "unknown")
        
        export GIT_REV GIT_BRANCH GIT_DESC
        print_info "Git info - Rev: $GIT_REV, Branch: $GIT_BRANCH, Desc: $GIT_DESC"
    else
        print_warning "Git not available, using default values"
        export GIT_REV="unknown"
        export GIT_BRANCH="unknown"
        export GIT_DESC="unknown"
    fi
}

# Check dependencies
check_dependencies() {
    print_info "Checking dependencies..."

    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed. Please install CMake 3.16 or higher."
        exit 1
    fi

    # Check for Qt6
    if ! pkg-config --exists Qt6Widgets; then
        print_warning "Qt6 development libraries not found via pkg-config."
        print_info "Make sure Qt6 development packages are installed."
        print_info "On Ubuntu/Debian: sudo apt install qt6-base-dev qt6-tools-dev"
        print_info "On Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
    fi

    # Check for required tools
    local required_tools=("make" "gcc" "g++")
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            print_error "$tool is not installed. Please install build-essential package."
            exit 1
        fi
    done

    print_success "Dependencies check completed"
}

# Clean build directory
clean_build() {
    print_info "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
    
    if [ -d "$DIST_DIR" ]; then
        rm -rf "$DIST_DIR"
        print_success "Distribution directory cleaned"
    fi
}

# Configure CMake for Linux
configure_linux() {
    print_info "Configuring for Linux build..."
    
    cmake -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=g++ \
        -DBUILD_GUI=ON \
        -DGIT_REV="$GIT_REV" \
        -DGIT_BRANCH="$GIT_BRANCH" \
        -DGIT_DESC="$GIT_DESC" \
        .
}

# Build for Linux
build_linux() {
    print_info "Building for Linux..."
    
    configure_linux
    cmake --build "$BUILD_DIR" --config Release -j$(nproc)
    
    if [ $? -eq 0 ]; then
        print_success "Linux build completed successfully"
        
        # Create distribution
        create_linux_dist
    else
        print_error "Linux build failed"
        exit 1
    fi
}

# Create Linux distribution
create_linux_dist() {
    local version=$(get_current_version)
    local dist_name="${PROJECT_NAME}-${version}-linux-x64"
    local dist_path="${DIST_DIR}/${dist_name}"
    
    print_info "Creating Linux distribution: $dist_name"
    
    mkdir -p "$dist_path"
    
    # Copy binaries
    if [ -f "${BUILD_DIR}/bin/shadps4-manager-gui" ]; then
        cp "${BUILD_DIR}/bin/shadps4-manager-gui" "$dist_path/"
        chmod +x "$dist_path/shadps4-manager-gui"
    fi
    
    if [ -f "${BUILD_DIR}/bin/shadps4-pkg-extractor" ]; then
        cp "${BUILD_DIR}/bin/shadps4-pkg-extractor" "$dist_path/"
        chmod +x "$dist_path/shadps4-pkg-extractor"
    fi
    
    if [ -f "${BUILD_DIR}/bin/shadps4-cli" ]; then
        cp "${BUILD_DIR}/bin/shadps4-cli" "$dist_path/"
        chmod +x "$dist_path/shadps4-cli"
    fi
    
    # Copy documentation
    cp README.md "$dist_path/" 2>/dev/null || true
    cp LICENSE "$dist_path/" 2>/dev/null || true
    
    # Create archive
    cd "$DIST_DIR"
    tar -czf "${dist_name}.tar.gz" "$dist_name"
    cd ..
    
    print_success "Linux distribution created: ${DIST_DIR}/${dist_name}.tar.gz"
}

# Main script logic
main() {
    print_info "ShadPs4Manager Build Script"
    print_info "Current directory: $(pwd)"
    
    # Parse arguments
    local version_action=""
    local build_target="linux"
    
    for arg in "$@"; do
        case $arg in
            major|minor|patch)
                version_action="$arg"
                ;;
            linux|windows|all)
                build_target="$arg"
                ;;
            clean)
                clean_build
                exit 0
                ;;
            help|--help|-h)
                echo "Usage: $0 [major|minor|patch] [linux|windows|all] [clean]"
                echo ""
                echo "Version actions:"
                echo "  major  - Increment major version (X.0.0)"
                echo "  minor  - Increment minor version (x.X.0)"
                echo "  patch  - Increment patch version (x.x.X)"
                echo ""
                echo "Build targets:"
                echo "  linux   - Build for Linux (default)"
                echo "  windows - Cross-compile for Windows"
                echo "  all     - Build for all platforms"
                echo ""
                echo "Other actions:"
                echo "  clean   - Clean build directories"
                echo "  help    - Show this help"
                exit 0
                ;;
        esac
    done
    
    # Handle version increment
    if [ -n "$version_action" ]; then
        local current_version=$(get_current_version)
        local new_version=$(increment_version "$current_version" "$version_action")
        
        print_info "Incrementing version: $current_version -> $new_version"
        update_cmake_version "$new_version"
        
        # Commit version change if git is available
        if command -v git &> /dev/null && [ -d ".git" ]; then
            git add "$CMAKE_VERSION_FILE" "src/common/scm_rev.cpp.in" "$VERSION_FILE" 2>/dev/null || true
            git commit -m "Bump version to $new_version" 2>/dev/null || true
            git tag "v$new_version" 2>/dev/null || true
            print_info "Version committed and tagged"
        fi
    fi
    
    # Setup
    get_git_info
    check_dependencies
    
    # Build based on target
    case $build_target in
        "linux")
            build_linux
            ;;
        *)
            print_error "Windows builds require GitHub Actions. Use 'linux' target for local builds."
            exit 1
            ;;
    esac
    
    print_success "Build script completed successfully!"
}

# Run main function
main "$@"

# Function to clean build directory
do_clean() {
    print_info "Cleaning build directory..."

    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_info "Build directory does not exist, nothing to clean"
    fi

    # Clean any other generated files if they exist
    if [ -f "CMakeCache.txt" ]; then
        rm -f CMakeCache.txt
    fi

    if [ -d "CMakeFiles" ]; then
        rm -rf CMakeFiles
    fi

    print_success "Clean completed"
}

# Function to configure CMake
do_configure() {
    print_info "Configuring CMake build system..."

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure with CMake
    cmake -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DBUILD_GUI=ON \
          ..

    if [ $? -eq 0 ]; then
        print_success "CMake configuration completed"
    else
        print_error "CMake configuration failed"
        exit 1
    fi

    cd ..
}

# Function to build the project
do_build() {
    print_info "Building project..."

    # Check if CMake configuration exists
    if [ ! -d "$BUILD_DIR" ]; then
        print_info "Build directory not found, configuring first..."
        do_configure
    fi

    cd "$BUILD_DIR"

    # Build with make
    make -j$(nproc)

    if [ $? -eq 0 ]; then
        print_success "Build completed successfully"
    else
        print_error "Build failed"
        exit 1
    fi

    cd ..
}

# Function to install the project
do_install() {
    print_info "Installing project..."

    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Please build the project first."
        exit 1
    fi

    cd "$BUILD_DIR"

    # Install with make
    sudo make install

    if [ $? -eq 0 ]; then
        print_success "Installation completed"
    else
        print_error "Installation failed"
        exit 1
    fi

    cd ..
}

# Function to run the application
do_run() {
    print_info "Running $PROJECT_NAME..."

    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Please build the project first."
        exit 1
    fi

    cd "$BUILD_DIR"

    # Check if the executable exists
    if [ ! -f "bin/shadps4-manager-gui" ]; then
        print_info "Executable not found, building first..."
        do_build
    fi

    # Run the application
    ./bin/shadps4-manager-gui

    cd ..
}

# Function to rebuild everything
do_rebuild() {
    print_info "Rebuilding project from scratch..."
    do_clean
    do_configure
    do_build
    print_success "Rebuild completed"
}

# Function to show project information
show_project_info() {
    echo ""
    echo -e "${PURPLE}Project Information:${NC}"
    echo -e "  Name: ${CYAN}$PROJECT_NAME${NC}"
    echo -e "  Type: ${CYAN}Qt6 GUI Application${NC}"
    echo -e "  Build System: ${CYAN}CMake${NC}"
    echo -e "  Features: ${CYAN}Game Library, Param.sfo Editor, PKG Support${NC}"
    echo ""
    echo -e "${PURPLE}System Information:${NC}"
    echo -e "  OS: ${CYAN}$(uname -s) $(uname -r)${NC}"
    echo -e "  CPU Cores: ${CYAN}$(nproc)${NC}"
    echo -e "  CMake Version: ${CYAN}$(cmake --version | head -n1)${NC}"
    echo ""
}

# Function to handle menu selection
handle_menu_choice() {
    case $1 in
        1)
            check_dependencies
            do_build
            ;;
        2)
            check_dependencies
            do_build
            echo ""
            do_run
            ;;
        3)
            do_clean
            ;;
        4)
            check_dependencies
            do_rebuild
            ;;
        5)
            check_dependencies
            do_configure
            ;;
        6)
            do_install
            ;;
        7)
            check_dependencies
            ;;
        8)
            show_project_info
            ;;
        q|Q)
            print_info "Goodbye!"
            exit 0
            ;;
        *)
            print_error "Invalid option. Please try again."
            return 1
            ;;
    esac
    return 0
}

# Interactive menu mode
run_interactive_menu() {
    while true; do
        print_header
        show_menu
        read -r choice

        if handle_menu_choice "$choice"; then
            echo ""
            echo -n "Press Enter to continue..."
            read -r
        fi
    done
}
