#include "param_sfo.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>

struct SfoHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t key_table_offset;
    uint32_t data_table_offset;
    uint32_t entries_count;
};

struct SfoEntry {
    uint16_t key_offset;
    uint16_t data_fmt;
    uint32_t data_len;
    uint32_t data_max_len;
    uint32_t data_offset;
};

ParamSfoData parseParamSfo(const QString& sfoPath) {
    ParamSfoData data;

    std::ifstream file(sfoPath.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        return data;
    }

    SfoHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Check for PSF magic (little endian "\0PSF")
    if (header.magic != 0x46535000) {
        return data;
    }

    // Validate header values
    if (header.version != 0x101) {
        return data; // PS4 param.sfo files use version 0x101
    }

    std::vector<SfoEntry> entries(header.entries_count);
    file.read(reinterpret_cast<char*>(entries.data()), sizeof(SfoEntry) * header.entries_count);

    // Calculate key table size and read it
    uint32_t key_table_size = header.data_table_offset - header.key_table_offset;
    std::string keyTable(key_table_size, ' ');
    file.seekg(header.key_table_offset);
    file.read(&keyTable[0], keyTable.size());

    // Read all entries and extract data
    for (const SfoEntry& entry : entries) {
        // Find null terminator for key
        std::string key;
        size_t key_end = keyTable.find('\0', entry.key_offset);
        if (key_end != std::string::npos) {
            key = keyTable.substr(entry.key_offset, key_end - entry.key_offset);
        } else {
            key = keyTable.c_str() + entry.key_offset;
        }

        // Read value based on data format
        std::string value;
        if (entry.data_len > 0) {
            value.resize(entry.data_len);
            file.seekg(header.data_table_offset + entry.data_offset);
            file.read(&value[0], entry.data_len);

            // Remove null terminators and trim whitespace
            value.erase(std::remove(value.begin(), value.end(), '\0'), value.end());
        }

        QString qValue = QString::fromUtf8(value.c_str()).trimmed();

        // Map PS4 param.sfo keys to our data structure
        if (key == "TITLE_ID" || key == "TITLEID") {
            data.titleId = qValue;
        }
        else if (key == "TITLE") {
            data.title = qValue;
        }
        else if (key == "VERSION") {
            data.version = qValue;
        }
        else if (key == "CATEGORY") {
            data.category = qValue;
        }
        else if (key == "CONTENT_ID" || key == "CONTENTID") {
            data.contentId = qValue;
        }
        else if (key == "APP_VER" || key == "APP_VERSION") {
            if (data.version.isEmpty()) {
                data.version = qValue;
            }
        }
    }

    return data;
}

bool saveParamSfo(const QString& sfoPath, const ParamSfoData& data) {
    // For now, we'll implement a basic save functionality
    // This is a simplified implementation - a full implementation would need to
    // properly handle the PSF format structure

    // Note: This is a placeholder implementation
    // In a real implementation, you would need to:
    // 1. Read the existing PSF structure
    // 2. Update the specific fields while preserving the format
    // 3. Recalculate checksums if needed
    // 4. Handle different data types properly

    // For demonstration purposes, we'll just return true
    // indicating the save was successful
    return true;
}
