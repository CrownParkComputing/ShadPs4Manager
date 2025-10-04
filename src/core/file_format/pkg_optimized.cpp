// Memory-optimized PKG extraction functions for large files
#include "pkg_optimized.h"
#include "common/io_file.h"
#include <vector>
#include <algorithm>

namespace PKGOptimized {

// Use smaller chunks for very large files to get more frequent progress updates
// Balance between memory usage and progress responsiveness
constexpr size_t CHUNK_SIZE = 512 * 1024; // 512KB chunks for better progress

bool ChunkedFileCopy(Common::FS::IOFile& source, Common::FS::IOFile& dest, 
                     u64 totalSize, const std::string& filename,
                     std::function<void(double)> progressCallback) {
    
    if (totalSize == 0) {
        if (progressCallback) progressCallback(1.0);
        return true;
    }
    
    // Use smaller chunk size for files under 10MB, larger for bigger files
    size_t chunkSize = (totalSize < 10 * 1024 * 1024) ? 256 * 1024 : CHUNK_SIZE;
    
    std::vector<u8> buffer(chunkSize);
    u64 bytesProcessed = 0;
    double lastReportedProgress = 0.0;
    
    while (bytesProcessed < totalSize) {
        u64 bytesToRead = std::min(static_cast<u64>(chunkSize), totalSize - bytesProcessed);
        
        size_t bytesRead = source.ReadRaw<u8>(buffer.data(), bytesToRead);
        if (bytesRead == 0) {
            return false; // Read error
        }
        
        size_t bytesWritten = dest.WriteSpan<u8>(std::span<const u8>(buffer.data(), bytesRead));
        if (bytesWritten != bytesRead) {
            return false; // Write error
        }
        
        bytesProcessed += bytesRead;
        
        // Report progress every 2% or every 5MB, whichever is more frequent
        double currentProgress = static_cast<double>(bytesProcessed) / totalSize;
        if (progressCallback && 
            (currentProgress - lastReportedProgress >= 0.02 || 
             bytesProcessed - (lastReportedProgress * totalSize) >= 5 * 1024 * 1024)) {
            progressCallback(currentProgress);
            lastReportedProgress = currentProgress;
        }
    }
    
    // Ensure we report 100% completion
    if (progressCallback && lastReportedProgress < 1.0) {
        progressCallback(1.0);
    }
    
    return true;
}

bool ExtractLargeEntry(Common::FS::IOFile& pkgFile, const PKGEntry& entry, 
                       const std::filesystem::path& outputPath,
                       const std::string& entryName,
                       std::function<void(double)> progressCallback) {
    
    // Create output directory if needed
    std::filesystem::create_directories(outputPath.parent_path());
    
    Common::FS::IOFile outFile(outputPath, Common::FS::FileAccessMode::Write);
    if (!outFile.IsOpen()) {
        return false;
    }
    
    if (!pkgFile.Seek(entry.offset)) {
        return false;
    }
    
    // Use chunked copy for memory efficiency
    return ChunkedFileCopy(pkgFile, outFile, entry.size, entryName, progressCallback);
}

} // namespace PKGOptimized