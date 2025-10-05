#!/bin/bash

# ShadPs4Manager Build Script with Interactive Menu
# This script provides convenient build, run, and clean operations for the ShadPs4Manager project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Project configuration
PROJECT_NAME="ShadPs4Manager"
BUILD_DIR="build"
CMAKE_BUILD_TYPE="Release"

# Function to print colored output
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

print_header() {
    echo -e "${PURPLE}================================${NC}"
    echo -e "${PURPLE}  ShadPs4Manager Build Menu${NC}"
    echo -e "${PURPLE}================================${NC}"
}

# Function to show main menu
show_menu() {
    echo ""
    echo "Please select an option:"
    echo ""
    echo -e "  ${CYAN}1)${NC} Build project"
    echo -e "  ${CYAN}2)${NC} Build and run"
    echo -e "  ${CYAN}3)${NC} Clean build"
    echo -e "  ${CYAN}4)${NC} Rebuild everything"
    echo -e "  ${CYAN}5)${NC} Configure CMake"
    echo -e "  ${CYAN}6)${NC} Install system-wide"
    echo -e "  ${CYAN}7)${NC} Check dependencies"
    echo -e "  ${CYAN}8)${NC} Show project info"
    echo ""
    echo -e "  ${RED}q)${NC} Quit"
    echo ""
    echo -n "Enter your choice [1-8 or q]: "
}

# Function to check dependencies
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

# Main script logic
main() {
    # If arguments provided, use command line mode
    if [ $# -gt 0 ]; then
        case "$1" in
            "clean")
                do_clean
                ;;
            "configure")
                check_dependencies
                do_configure
                ;;
            "build")
                check_dependencies
                do_build
                ;;
            "install")
                do_install
                ;;
            "run")
                check_dependencies
                do_run
                ;;
            "rebuild")
                check_dependencies
                do_rebuild
                ;;
            "menu")
                run_interactive_menu
                ;;
            "help"|"-h"|"--help")
                print_header
                echo "Usage: $0 [OPTION]"
                echo ""
                echo "Command Line Options:"
                echo "  build     Build the project"
                echo "  run       Build and run the application"
                echo "  clean     Clean build directory"
                echo "  rebuild   Clean and rebuild everything"
                echo "  configure Configure CMake build system"
                echo "  install   Install system-wide"
                echo "  menu      Start interactive menu mode"
                echo "  help      Show this help message"
                echo ""
                echo "Interactive Menu:"
                echo "  Run $0 without arguments or use '$0 menu' for interactive mode"
                echo ""
                ;;
            *)
                print_error "Unknown option: $1"
                echo "Use '$0 help' for usage information"
                exit 1
                ;;
        esac
    else
        # No arguments provided, show interactive menu
        run_interactive_menu
    fi
}

# Run the main function with all arguments
main "$@"
