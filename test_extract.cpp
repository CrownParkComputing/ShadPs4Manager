#include <iostream>
#include <filesystem>
#include "src/core/file_format/pkg.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <pkg_file> <output_dir>" << std::endl;
        return 1;
    }

    std::filesystem::path pkgPath = argv[1];
    std::filesystem::path outputPath = argv[2];
    
    if (!std::filesystem::exists(pkgPath)) {
        std::cout << "PKG file not found: " << pkgPath << std::endl;
        return 1;
    }

    PKG pkg;
    std::string failreason;
    
    std::cout << "Opening PKG file: " << pkgPath << std::endl;
    if (!pkg.Open(pkgPath, failreason)) {
        std::cout << "Failed to open PKG: " << failreason << std::endl;
        return 1;
    }
    
    std::cout << "PKG opened successfully" << std::endl;
    std::cout << "Extracting to: " << outputPath << std::endl;
    
    if (!pkg.Extract(outputPath, pkgPath, failreason)) {
        std::cout << "Extraction failed: " << failreason << std::endl;
        return 1;
    }
    
    std::cout << "Extraction completed successfully" << std::endl;
    return 0;
}