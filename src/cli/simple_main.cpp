#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::cout << "ShadPs4Manager CLI v1.0.0\n";
    std::cout << "PS4 PKG File Extractor\n\n";

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <path/to/pkg> [path/to/output]\n";
        std::cout << "   OR: " << argv[0] << " --dir <directory/with/pkgs> [path/to/output]\n";
        std::cout << "       If output path is omitted, the PKG will be extracted to its parent directory\n";
        return 1;
    }

    if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        std::cout << "ShadPs4Manager - PS4 PKG File Extractor\n\n";
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " <pkg_file> [output_directory]\n";
        std::cout << "  " << argv[0] << " --dir <directory> [output_directory]\n";
        std::cout << "  " << argv[0] << " --help\n\n";
        std::cout << "Arguments:\n";
        std::cout << "  pkg_file          Path to PS4 PKG file to extract\n";
        std::cout << "  directory         Directory containing PKG files (with --dir)\n";
        std::cout << "  output_directory  Directory to extract files to (optional)\n\n";
        std::cout << "Examples:\n";
        std::cout << "  " << argv[0] << " game.pkg\n";
        std::cout << "  " << argv[0] << " game.pkg /tmp/extracted\n";
        std::cout << "  " << argv[0] << " --dir /games/folder /tmp/output\n";
        return 0;
    }

    // For now, this is a placeholder that shows the interface
    // In a real implementation, this would use the extraction library
    std::cout << "PKG file: " << argv[1] << "\n";
    
    if (argc > 2) {
        std::cout << "Output directory: " << argv[2] << "\n";
    } else {
        std::filesystem::path pkg_path(argv[1]);
        std::cout << "Output directory: " << pkg_path.parent_path() << "/" << pkg_path.stem() << "_extracted\n";
    }
    
    std::cout << "\nNote: This is a demonstration version.\n";
    std::cout << "For actual PKG extraction, use the GUI application or the working CLI tool.\n";
    
    return 0;
}