// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <string>
#include <filesystem>

class UpdateMerger {
public:
    struct MergeResult {
        bool success = false;
        std::string errorMessage;
        int filesOverwritten = 0;
        int filesAdded = 0;
        uint64_t totalSize = 0;
    };

    static MergeResult mergeUpdateToBaseGame(
        const std::string& tempUpdatePath,
        const std::string& baseGamePath,
        bool deleteSourceAfterMerge = true
    );

private:
    static bool mergeDirectoryRecursive(
        const std::filesystem::path& source,
        const std::filesystem::path& destination,
        MergeResult& result
    );

    static bool shouldOverwriteFile(
        const std::filesystem::path& sourceFile,
        const std::filesystem::path& destFile
    );
};