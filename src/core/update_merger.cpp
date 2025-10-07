// SPDX-License-Identifier: GPL-2.0-or-later
#include "update_merger.h"
#include <filesystem>
#include <fstream>
#include <iostream>

UpdateMerger::MergeResult UpdateMerger::mergeUpdateToBaseGame(
    const std::string& tempUpdatePath,
    const std::string& baseGamePath,
    bool deleteSourceAfterMerge) {
    
    MergeResult result;
    
    try {
        std::filesystem::path sourcePath(tempUpdatePath);
        std::filesystem::path destPath(baseGamePath);
        
        // Validate paths
        if (!std::filesystem::exists(sourcePath)) {
            result.errorMessage = "Source update directory does not exist: " + tempUpdatePath;
            return result;
        }
        
        if (!std::filesystem::exists(destPath)) {
            result.errorMessage = "Base game directory does not exist: " + baseGamePath;
            return result;
        }
        
        std::cout << "Starting update merge from " << tempUpdatePath << " to " << baseGamePath << std::endl;
        
        // Perform recursive merge
        if (!mergeDirectoryRecursive(sourcePath, destPath, result)) {
            return result;
        }
        
        // Clean up source directory if requested and merge was successful
        if (deleteSourceAfterMerge && result.success) {
            try {
                std::filesystem::remove_all(sourcePath);
                std::cout << "Cleaned up temporary update directory: " << tempUpdatePath << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Warning: Failed to clean up temporary directory: " << e.what() << std::endl;
                // Don't fail the merge for cleanup issues
            }
        }
        
        result.success = true;
        std::cout << "Update merge completed successfully: "
                 << result.filesAdded << " files added, "
                 << result.filesOverwritten << " files overwritten, "
                 << "total size: " << result.totalSize << " bytes" << std::endl;
        
    } catch (const std::exception& e) {
        result.errorMessage = std::string("Error during update merge: ") + e.what();
        std::cout << "Update merge failed: " << result.errorMessage << std::endl;
    }
    
    return result;
}

bool UpdateMerger::mergeDirectoryRecursive(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    MergeResult& result) {
    
    try {
        // Ensure destination directory exists
        std::filesystem::create_directories(destination);
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
            if (entry.is_regular_file()) {
                // Calculate relative path from source
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), source);
                std::filesystem::path destFile = destination / relativePath;
                
                // Ensure destination directory exists
                std::filesystem::create_directories(destFile.parent_path());
                
                bool fileExists = std::filesystem::exists(destFile);
                
                // Determine if we should overwrite
                if (!fileExists || shouldOverwriteFile(entry.path(), destFile)) {
                    try {
                        // Copy file, overwriting if it exists
                        std::filesystem::copy_file(entry.path(), destFile, 
                                                 std::filesystem::copy_options::overwrite_existing);
                        
                        result.totalSize += std::filesystem::file_size(entry.path());
                        
                        if (fileExists) {
                            result.filesOverwritten++;
                            std::cout << "Overwritten: " << relativePath.string() << std::endl;
                        } else {
                            result.filesAdded++;
                            std::cout << "Added: " << relativePath.string() << std::endl;
                        }
                        
                    } catch (const std::exception& e) {
                        result.errorMessage = std::string("Failed to copy file ") + relativePath.string() + ": " + e.what();
                        return false;
                    }
                } else {
                    std::cout << "Skipped (older or same): " << relativePath.string() << std::endl;
                }
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        result.errorMessage = std::string("Error during directory merge: ") + e.what();
        return false;
    }
}

bool UpdateMerger::shouldOverwriteFile(
    const std::filesystem::path& sourceFile,
    const std::filesystem::path& destFile) {
    
    try {
        // If destination doesn't exist, always copy
        if (!std::filesystem::exists(destFile)) {
            return true;
        }
        
        // Compare file sizes first (quick check)
        auto sourceSize = std::filesystem::file_size(sourceFile);
        auto destSize = std::filesystem::file_size(destFile);
        
        if (sourceSize != destSize) {
            return true; // Different sizes, likely different files
        }
        
        // Compare last write times
        auto sourceTime = std::filesystem::last_write_time(sourceFile);
        auto destTime = std::filesystem::last_write_time(destFile);
        
        // Overwrite if source is newer or same time (update files should be newer or equal)
        return sourceTime >= destTime;
        
    } catch (const std::exception& e) {
        std::cout << "Error comparing files, defaulting to overwrite: " << e.what() << std::endl;
        return true; // When in doubt, overwrite
    }
}