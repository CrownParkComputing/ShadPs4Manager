// ShadPs4 Manager - PKG Extraction CLI Tool
// 
// PURPOSE:
// Standalone command-line tool for extracting PS4 PKG files.
// Can be used independently or called by the GUI application.
// 
// USAGE:
//   shadps4-pkg-extractor <pkg_file> [output_dir]
//   shadps4-pkg-extractor --dir <directory> [output_dir]
// 
// FEATURES:
// - Real-time progress output to stdout
// - Graceful signal handling (SIGINT/SIGTERM)
// - Enhanced error handling with detailed messages
// - Continues extraction on individual file errors
// - Periodic heartbeat messages for long-running operations
// 
// INTEGRATION:
// When called by the GUI, stdout/stderr are captured and displayed
// in real-time in the Extraction Log tab. Process isolation ensures
// crashes don't affect the GUI application.

// SPDX-License-Identifier: GPL-2.0-or-later
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <csignal>
#include "core/file_format/pkg.h"

// Global flag for graceful termination on signals
static std::atomic<bool> g_terminate{false};

void signal_handler(int signum) {
    std::cerr << "\n[signal] Received signal " << signum << ", terminating gracefully..." << std::endl;
    g_terminate.store(true);
}

bool ProcessPkg(const std::filesystem::path& pkgPath, const std::filesystem::path& outDir) {
    std::cout << "\nProcessing PKG: " << pkgPath.filename() << std::endl;
    
    if (!std::filesystem::exists(pkgPath)) {
        std::cerr << "PKG file does not exist: " << pkgPath << std::endl;
        return false;
    }
    
    PKG pkg;
    std::string failReason;

    // Wire progress callback to print live updates
    auto start_time = std::chrono::steady_clock::now();
    pkg.SetProgressCallback([&](const PKGProgress& pr) {
        using Stage = PKGProgress::Stage;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        switch (pr.stage) {
            case Stage::Opening:
                std::cout << "Opening..." << std::endl; break;
            case Stage::ReadingMetadata: {
                std::cout << "Metadata parsed." << std::endl; break;
            }
            case Stage::ParsingPFS:
                if (!pr.message.empty()) {
                    std::cout << "Parsing PFS: " << pr.message << std::endl;
                } else {
                    std::cout << "Parsing PFS..." << std::endl;
                }
                break;
            case Stage::Extracting: {
                std::cout << "\r[" << std::fixed << std::setprecision(1) << pr.percent
                          << "%] Files " << pr.files_done << "/" << pr.files_total
                          << ", Bytes " << pr.bytes_done << "/" << pr.bytes_total
                          << ": " << pr.current_file << "  Elapsed: " << elapsed << "s" << std::flush;
                break;
            }
            case Stage::Done:
                std::cout << "\nDone." << std::endl; break;
            case Stage::Error:
                std::cout << "\nError: " << pr.message << std::endl; break;
        }
    });

    // Open and analyze the PKG for metadata
    if (!pkg.Open(pkgPath, failReason)) {
        std::cerr << "Failed to open PKG: " << failReason << std::endl;
        return false;
    }

    // Show metadata
    const PKGMeta meta = pkg.GetMetadata();
    std::cout << "Content ID: " << meta.content_id << std::endl;
    std::cout << "Title ID:   " << meta.title_id << std::endl;
    std::cout << "PKG Type:   0x" << std::hex << meta.pkg_type << std::dec << std::endl;
    std::cout << "Content Type: 0x" << std::hex << meta.content_type << std::dec << std::endl;
    std::cout << "Flags:      0x" << std::hex << meta.content_flags << std::dec;
    if (!meta.content_flag_names.empty()) {
        std::cout << " (";
        for (size_t i = 0; i < meta.content_flag_names.size(); ++i) {
            std::cout << meta.content_flag_names[i];
            if (i + 1 < meta.content_flag_names.size()) std::cout << ",";
        }
        std::cout << ")";
    }
    std::cout << std::endl;
    std::cout << "Sizes: pkg=" << meta.pkg_size << ", body=" << meta.body_size
              << ", content=" << meta.content_size << ", pfs_image=" << meta.pfs_image_size << std::endl;
    std::cout << "PKG table entries: " << meta.file_count << std::endl;

    // Create output directory (use provided path as exact root)
    std::filesystem::path actualOutDir = outDir;
    if (actualOutDir.empty()) actualOutDir = pkgPath.parent_path();
    std::cout << "Extracting to: " << actualOutDir << std::endl;
    try {
        std::filesystem::create_directories(actualOutDir);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create output directory: " << e.what() << std::endl;
        return false;
    }

    // Perform extraction (parses PFS and extracts all files with progress)
    // Enhanced error handling for crashes
    std::cout << "Starting PFS extraction..." << std::endl;
    std::cout.flush();
    try {
        if (!pkg.Extract(pkgPath, actualOutDir, failReason)) {
            std::cerr << "Extraction failed: " << failReason << std::endl;
            return false;
        }
    } catch (const std::out_of_range& e) {
        std::cerr << "FATAL: Out of range error during extraction (corrupted PKG data): " << e.what() << std::endl;
        return false;
    } catch (const std::runtime_error& e) {
        std::cerr << "FATAL: Runtime error during extraction: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "FATAL: Exception during extraction: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "FATAL: Unknown exception during extraction (possibly PFS parsing error)" << std::endl;
        std::cerr << "This may indicate corrupted PKG or unsupported PKG format." << std::endl;
        return false;
    }

    // Check for termination signal
    if (g_terminate.load()) {
        std::cerr << "Extraction cancelled by signal" << std::endl;
        return false;
    }

    // Now extract all the actual files from the PFS
    u32 total_files = pkg.GetNumberOfFiles();
    std::cout << "\nExtracting " << total_files << " PFS entries..." << std::endl;
    std::cout.flush();
    
    int files_extracted = 0;
    for (u32 i = 0; i < total_files; ++i) {
        // Check termination signal
        if (g_terminate.load()) {
            std::cerr << "\nExtraction cancelled by signal at file " << i << std::endl;
            return false;
        }
        
        try {
            // ExtractFiles will only extract if it's a file (not directory)
            pkg.ExtractFiles(static_cast<int>(i));
            files_extracted++;
            
            // Periodic heartbeat every 100 files
            if (i % 100 == 0 && i > 0) {
                std::cout << "[progress] Extracted " << i << "/" << total_files << " entries..." << std::endl;
                std::cout.flush();
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "\nError: Out of range extracting file index " << i << ": " << e.what() << std::endl;
            // Continue with next file instead of aborting
        } catch (const std::runtime_error& e) {
            std::cerr << "\nError: Runtime error extracting file index " << i << ": " << e.what() << std::endl;
            // Continue with next file
        } catch (const std::exception& e) {
            std::cerr << "\nWarning: Failed to extract file index " << i << ": " << e.what() << std::endl;
            // Continue with next file
        } catch (...) {
            std::cerr << "\nWarning: Unknown error extracting file index " << i << std::endl;
            // Continue with next file
        }
    }
    
    std::cout << "\nExtracted " << files_extracted << " files successfully." << std::endl;
    std::cout << "Extraction completed." << std::endl;
    std::cout.flush();
    return true;
}

std::vector<std::filesystem::path> ListPkgFiles(const std::filesystem::path& dir, bool recursive) {
    std::vector<std::filesystem::path> pkgFiles;
    
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return pkgFiles;
    }
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".pkg") {
                    pkgFiles.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".pkg") {
                    pkgFiles.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }
    
    return pkgFiles;
}

int main(int argc, char** argv) {
    // Register signal handlers for graceful termination
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    std::cout << "ShadPs4Manager PKG Extraction CLI Tool" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout.flush();
    
    // Handle help flags
    if (argc < 2 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << argv[0] << " <pkg_file> [output_dir]" << std::endl;
        std::cout << "  " << argv[0] << " --dir <directory> [output_dir]" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --dir     Process all PKG files in directory recursively" << std::endl;
        std::cout << "  -h, --help  Show this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "If output_dir is not specified, files will be extracted to the PKG's parent directory." << std::endl;
        return 0;
    }
    
    // Directory mode
    if (argc >= 3 && std::string(argv[1]) == "--dir") {
        std::filesystem::path sourceDir = argv[2];
        std::filesystem::path outputDir = (argc >= 4) ? argv[3] : sourceDir;
        
        if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
            std::cerr << "Error: Source directory not found: " << sourceDir << std::endl;
            return 1;
        }
        
        std::cout << "Searching for PKG files in: " << sourceDir << std::endl;
        auto pkgFiles = ListPkgFiles(sourceDir, true);
        
        if (pkgFiles.empty()) {
            std::cout << "No PKG files found in the specified directory." << std::endl;
            return 0;
        }
        
        std::cout << "Found " << pkgFiles.size() << " PKG files to process." << std::endl;
        
        int successCount = 0;
        int failedCount = 0;
        
        for (size_t i = 0; i < pkgFiles.size(); ++i) {
            const auto& pkgPath = pkgFiles[i];
            std::cout << "\n[" << (i + 1) << "/" << pkgFiles.size() << "] Processing " 
                      << pkgPath.filename() << std::endl;
            
            if (ProcessPkg(pkgPath, outputDir)) {
                successCount++;
            } else {
                failedCount++;
            }
        }
        
        std::cout << "\nBatch processing complete: " << successCount << " successful, " 
                  << failedCount << " failed." << std::endl;
        
        return failedCount > 0 ? 1 : 0;
    }
    // Single file mode
    else {
        std::filesystem::path pkgPath = argv[1];
        std::filesystem::path outputDir = (argc >= 3) ? argv[2] : pkgPath.parent_path();
        
        if (outputDir.empty()) {
            outputDir = std::filesystem::current_path();
        }
        
        return ProcessPkg(pkgPath, outputDir) ? 0 : 1;
    }
}