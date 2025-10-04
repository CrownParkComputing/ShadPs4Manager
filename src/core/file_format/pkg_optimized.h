// Memory-optimized PKG extraction functions for large files
#pragma once

#include "core/file_format/pkg.h"
#include "common/io_file.h"
#include <functional>
#include <filesystem>

namespace PKGOptimized {

// Chunked file copy with progress reporting - optimized for large files
bool ChunkedFileCopy(Common::FS::IOFile& source, Common::FS::IOFile& dest, 
                     u64 totalSize, const std::string& filename,
                     std::function<void(double)> progressCallback);

// Memory-optimized entry extraction for large PKG entries
bool ExtractLargeEntry(Common::FS::IOFile& pkgFile, const PKGEntry& entry, 
                       const std::filesystem::path& outputPath,
                       const std::string& entryName,
                       std::function<void(double)> progressCallback);

} // namespace PKGOptimized