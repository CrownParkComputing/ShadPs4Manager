#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <vector>
#include "pkg_tool/lib.h"

void printUsage(const char* programName) {
    std::cout << "ShadPs4Manager - PS4 PKG File Extractor\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " <pkg_file> [output_directory]\n";
    std::cout << "  " << programName << " --dir <directory> [output_directory]\n";
    std::cout << "  " << programName << " --help\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  pkg_file          Path to PS4 PKG file to extract\n";
    std::cout << "  directory         Directory containing PKG files (with --dir)\n";
    std::cout << "  output_directory  Directory to extract files to (optional)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " game.pkg\n";
    std::cout << "  " << programName << " game.pkg /tmp/extracted\n";
    std::cout << "  " << programName << " --dir /games/folder /tmp/output\n";
}

bool extractSinglePkg(const std::filesystem::path& pkgPath, const std::filesystem::path& outputPath) {
    std::cout << "Extracting PKG: " << pkgPath << "\n";
    std::cout << "Output directory: " << outputPath << "\n\n";
    
    // Read metadata first
    PkgMetadata metadata;
    auto metadataError = ReadPkgMetadata(pkgPath, metadata);
    if (metadataError) {
        std::cerr << "Error reading PKG metadata: " << *metadataError << "\n";
        return false;
    }
    
    std::cout << "PKG Information:\n";
    std::cout << "  Title ID: " << metadata.title_id << "\n";
    std::cout << "  File Count: " << metadata.file_count << "\n";
    std::cout << "  PKG Size: " << (metadata.pkg_size / (1024 * 1024)) << " MB\n\n";
    
    // Progress callback for CLI output with improved responsiveness
    auto progressCallback = [](const ExtractionProgress& progress) {
        // Print progress updates more frequently for large files
        static int lastReportedPercent = -1;
        int currentPercent = static_cast<int>(progress.total_progress * 100);
        
        // Report every 1% for overall progress, or if we have file progress
        if (currentPercent != lastReportedPercent || 
            (progress.file_progress > 0.0 && progress.file_progress < 1.0)) {
            
            if (progress.file_progress > 0.0 && progress.file_progress < 1.0) {
                // Show intra-file progress for large files
                std::cout << "\rProgress: " << currentPercent 
                          << "% - " << progress.current_file 
                          << " (" << static_cast<int>(progress.file_progress * 100) << "%) "
                          << "(" << progress.current_file_index + 1 << "/" << progress.total_files 
                          << " files)" << std::flush;
            } else {
                std::cout << "\rProgress: " << currentPercent 
                          << "% (" << progress.current_file_index << "/" << progress.total_files 
                          << " files)" << std::flush;
            }
            lastReportedPercent = currentPercent;
        }
    };
    
    // Extract all files
    std::vector<int> indices; // Empty = extract all
    auto extractError = ExtractPkg(pkgPath, outputPath, indices, progressCallback);
    
    if (extractError) {
        std::cerr << "Error during extraction: " << *extractError << "\n";
        return false;
    }
    
    std::cout << "Extraction completed successfully!\n";
    return true;
}

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
        printUsage(argv[0]);
        return 0;
    }

    try {
        if (std::string(argv[1]) == "--dir") {
            if (argc < 3) {
                std::cerr << "Error: --dir option requires a directory path\n";
                return 1;
            }
            
            std::filesystem::path inputDir(argv[2]);
            std::filesystem::path outputDir;
            
            if (argc > 3) {
                outputDir = argv[3];
            } else {
                outputDir = inputDir / "extracted";
            }
            
            // Find all PKG files in the directory
            auto pkgFiles = ListPkgFiles(inputDir, false); // non-recursive
            
            if (pkgFiles.empty()) {
                std::cout << "No PKG files found in directory: " << inputDir << "\n";
                return 1;
            }
            
            std::cout << "Found " << pkgFiles.size() << " PKG file(s) in directory\n\n";
            
            bool allSuccessful = true;
            for (const auto& pkgFile : pkgFiles) {
                std::filesystem::path pkgOutputDir = outputDir / pkgFile.stem();
                
                std::cout << "=== Processing: " << pkgFile.filename() << " ===\n";
                if (!extractSinglePkg(pkgFile, pkgOutputDir)) {
                    allSuccessful = false;
                    std::cerr << "Failed to extract: " << pkgFile << "\n\n";
                } else {
                    std::cout << "Successfully extracted: " << pkgFile.filename() << "\n\n";
                }
            }
            
            return allSuccessful ? 0 : 1;
            
        } else {
            // Single PKG file mode
            std::filesystem::path pkgPath(argv[1]);
            std::filesystem::path outputPath;
            
            if (argc > 2) {
                outputPath = argv[2];
            } else {
                outputPath = pkgPath.parent_path() / (pkgPath.stem().string() + "_extracted");
            }
            
            return extractSinglePkg(pkgPath, outputPath) ? 0 : 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
}