// SPDX-FileCopyrightText: Copyright 2024 shadPS4 Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>
#include "zlib/zlib.h"
#include "common/io_file.h"
#include "core/file_format/pkg.h"
#include "core/file_format/pkg_type.h"
#include "core/file_format/pkg_optimized.h"

static bool StreamingDecompressPFSC(std::span<char> compressed_data, std::span<char> decompressed_data, bool& stream_initialized, z_stream& stream) {
    if (!stream_initialized) {
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;

        if (inflateInit(&stream) != Z_OK) {
            return false;
        }
        stream_initialized = true;
    }

    stream.avail_in = compressed_data.size();
    stream.next_in = reinterpret_cast<unsigned char*>(compressed_data.data());
    stream.avail_out = decompressed_data.size();
    stream.next_out = reinterpret_cast<unsigned char*>(decompressed_data.data());

    int ret = inflate(&stream, Z_NO_FLUSH);
    if (ret < 0) {
        inflateEnd(&stream);
        stream_initialized = false;
        return false;
    }

    return true;
}

static bool DecompressPFSC(std::span<char> compressed_data, std::span<char> decompressed_data) {
    z_stream decompressStream;
    decompressStream.zalloc = Z_NULL;
    decompressStream.zfree = Z_NULL;
    decompressStream.opaque = Z_NULL;

    if (inflateInit(&decompressStream) != Z_OK) {
        return false; // Failed to initialize
    }

    decompressStream.avail_in = compressed_data.size();
    decompressStream.next_in = reinterpret_cast<unsigned char*>(compressed_data.data());
    decompressStream.avail_out = decompressed_data.size();
    decompressStream.next_out = reinterpret_cast<unsigned char*>(decompressed_data.data());

    int result = inflate(&decompressStream, Z_FINISH);
    bool success = (result == Z_STREAM_END);
    
    if (inflateEnd(&decompressStream) != Z_OK) {
        success = false;
    }
    
    return success;
}

u32 GetPFSCOffset(std::span<const u8> pfs_image) {
    static constexpr u32 PfscMagic = 0x43534650;
    u32 value;
    for (u32 i = 0x20000; i < pfs_image.size(); i += 0x10000) {
        std::memcpy(&value, &pfs_image[i], sizeof(u32));
        if (value == PfscMagic)
            return i;
    }
    return -1;
}

PKG::PKG() = default;

PKG::~PKG() = default;

void PKG::SetProgressCallback(std::function<void(const PKGProgress&)> cb) {
    progress_cb_ = std::move(cb);
}

void PKG::reportProgress(const PKGProgress& p) const {
    if (progress_cb_) progress_cb_(p);
}

std::vector<std::string> PKG::FlagsToNames(u32 flags) {
    std::vector<std::string> names;
    for (const auto& [flag, name] : flagNames) {
        if ((flags & static_cast<u32>(flag)) != 0) names.emplace_back(name);
    }
    return names;
}

PKGMeta PKG::GetMetadata() const {
    PKGMeta m{};
    // Content ID is 0x24 bytes; trim trailing nulls
    m.content_id.assign(reinterpret_cast<const char*>(pkgheader.pkg_content_id),
                        reinterpret_cast<const char*>(pkgheader.pkg_content_id) + 0x24);
    if (auto pos = m.content_id.find('\0'); pos != std::string::npos) m.content_id.resize(pos);

    m.title_id = std::string(pkgTitleID, pkgTitleID + 9);
    m.pkg_type = static_cast<u32>(pkgheader.pkg_type);
    m.content_type = static_cast<u32>(pkgheader.pkg_content_type);
    m.content_flags = static_cast<u32>(pkgheader.pkg_content_flags);
    m.content_flag_names = FlagsToNames(m.content_flags);
    m.pkg_size = pkgSize ? pkgSize : static_cast<u64>(pkgheader.pkg_size);
    m.body_size = static_cast<u64>(pkgheader.pkg_body_size);
    m.content_size = static_cast<u64>(pkgheader.pkg_content_size);
    m.pfs_image_size = static_cast<u64>(pkgheader.pfs_image_size);
    m.file_count = static_cast<u32>(pkgheader.pkg_file_count);
    return m;
}

bool PKG::Open(const std::filesystem::path& filepath, std::string& failreason) {
    PKGProgress prog{};
    prog.stage = PKGProgress::Stage::Opening;
    prog.message = "Opening PKG";
    reportProgress(prog);

    Common::FS::IOFile file(filepath, Common::FS::FileAccessMode::Read);
    if (!file.IsOpen()) {
        prog.stage = PKGProgress::Stage::Error;
        prog.message = "Failed to open PKG";
        reportProgress(prog);
        return false;
    }
    pkgSize = file.GetSize();

    file.Read(pkgheader);
    if (pkgheader.magic != 0x7F434E54)
        return false;

    for (const auto& flag : flagNames) {
        if (isFlagSet(pkgheader.pkg_content_flags, flag.first)) {
            if (!pkgFlags.empty())
                pkgFlags += (", ");
            pkgFlags += (flag.second);
        }
    }

    // Find title id it is part of pkg_content_id starting at offset 0x40
    file.Seek(0x47); // skip first 7 characters of content_id
    file.Read(pkgTitleID);

    u32 offset = pkgheader.pkg_table_entry_offset;
    u32 n_files = pkgheader.pkg_table_entry_count;

    if (!file.Seek(offset)) {
        failreason = "Failed to seek to PKG table entry offset";
        return false;
    }

    for (int i = 0; i < n_files; i++) {
        PKGEntry entry{};
        file.Read(entry.id);
        file.Read(entry.filename_offset);
        file.Read(entry.flags1);
        file.Read(entry.flags2);
        file.Read(entry.offset);
        file.Read(entry.size);
        file.Seek(8, Common::FS::SeekOrigin::CurrentPosition);

        // Try to figure out the name
        const auto name = GetEntryNameByType(entry.id);
        if (name == "param.sfo") {
            sfo.clear();
            if (!file.Seek(entry.offset)) {
                failreason = "Failed to seek to param.sfo offset";
                return false;
            }
            sfo.resize(entry.size);
            file.ReadRaw<u8>(sfo.data(), entry.size);
        }
    }
    file.Close();

    // Metadata available
    prog.stage = PKGProgress::Stage::ReadingMetadata;
    prog.message = "Metadata parsed";
    reportProgress(prog);

    return true;
}

bool PKG::Extract(const std::filesystem::path& filepath, const std::filesystem::path& extract,
                  std::string& failreason) {
    PKGProgress prog{};
    prog.stage = PKGProgress::Stage::ParsingPFS;
    prog.message = "Preparing extraction";
    reportProgress(prog);

    extract_path = extract;
    pkgpath = filepath;
    Common::FS::IOFile file(filepath, Common::FS::FileAccessMode::Read);
    if (!file.IsOpen()) {
        prog.stage = PKGProgress::Stage::Error;
        prog.message = "Failed to open PKG";
        reportProgress(prog);
        return false;
    }
    pkgSize = file.GetSize();
    file.ReadRaw<u8>(&pkgheader, sizeof(PKGHeader));

    if (pkgheader.magic != 0x7F434E54)
        return false;

    if (pkgheader.pkg_size > pkgSize) {
        failreason = "PKG file size is different";
        prog.stage = PKGProgress::Stage::Error;
        reportProgress(prog);
        return false;
    }
    if ((pkgheader.pkg_content_size + pkgheader.pkg_content_offset) > pkgheader.pkg_size) {
        failreason = "Content size is bigger than pkg size";
        prog.stage = PKGProgress::Stage::Error;
        reportProgress(prog);
        return false;
    }

    u32 offset = pkgheader.pkg_table_entry_offset;
    u32 n_files = pkgheader.pkg_table_entry_count;

    std::array<u8, 64> concatenated_ivkey_dk3;
    std::array<u8, 32> seed_digest;
    std::array<std::array<u8, 32>, 7> digest1;
    std::array<std::array<u8, 256>, 7> key1;
    std::array<u8, 256> imgkeydata;

    if (!file.Seek(offset)) {
        failreason = "Failed to seek to PKG table entry offset";
        return false;
    }

    for (int i = 0; i < n_files; i++) {
        PKGEntry entry{};
        file.Read(entry.id);
        file.Read(entry.filename_offset);
        file.Read(entry.flags1);
        file.Read(entry.flags2);
        file.Read(entry.offset);
        file.Read(entry.size);
        file.Seek(8, Common::FS::SeekOrigin::CurrentPosition);

        auto currentPos = file.Tell();

        // Try to figure out the name
        const auto name = GetEntryNameByType(entry.id);
        const auto filepath = extract_path / "sce_sys" / name;
        std::filesystem::create_directories(filepath.parent_path());

        if (name.empty()) {
            // Just print with id
            Common::FS::IOFile out(extract_path / "sce_sys" / std::to_string(entry.id),
                                   Common::FS::FileAccessMode::Write);
            if (!file.Seek(entry.offset)) {
                failreason = "Failed to seek to PKG entry offset";
                return false;
            }

            std::vector<u8> data;
            data.resize(entry.size);
            file.ReadRaw<u8>(data.data(), entry.size);
            out.WriteRaw<u8>(data.data(), entry.size);
            out.Close();

            file.Seek(currentPos);
            continue;
        }

        if (entry.id == 0x1) {         // DIGESTS, seek;
                                       // file.Seek(entry.offset, fsSeekSet);
        } else if (entry.id == 0x10) { // ENTRY_KEYS, seek;
            file.Seek(entry.offset);
            file.Read(seed_digest);

            for (int i = 0; i < 7; i++) {
                file.Read(digest1[i]);
            }

            for (int i = 0; i < 7; i++) {
                file.Read(key1[i]);
            }

            PKG::crypto.RSA2048Decrypt(dk3_, key1[3], true); // decrypt DK3
        } else if (entry.id == 0x20) {                       // IMAGE_KEY, seek; IV_KEY
            file.Seek(entry.offset);
            file.Read(imgkeydata);

            // The Concatenated iv + dk3 imagekey for HASH256
            std::memcpy(concatenated_ivkey_dk3.data(), &entry, sizeof(entry));
            std::memcpy(concatenated_ivkey_dk3.data() + sizeof(entry), dk3_.data(), sizeof(dk3_));

            PKG::crypto.ivKeyHASH256(concatenated_ivkey_dk3, ivKey); // ivkey_
            // imgkey_ to use for last step to get ekpfs
            PKG::crypto.aesCbcCfb128Decrypt(ivKey, imgkeydata, imgKey);
            // ekpfs key to get data and tweak keys.
            PKG::crypto.RSA2048Decrypt(ekpfsKey, imgKey, false);
        } else if (entry.id == 0x80) {
            // GENERAL_DIGESTS, seek;
            // file.Seek(entry.offset, fsSeekSet);
        }

        Common::FS::IOFile out(extract_path / "sce_sys" / name, Common::FS::FileAccessMode::Write);
        if (!file.Seek(entry.offset)) {
            failreason = "Failed to seek to PKG entry offset";
            return false;
        }

        std::vector<u8> data;
        data.resize(entry.size);
        file.ReadRaw<u8>(data.data(), entry.size);
        out.WriteRaw<u8>(data.data(), entry.size);
        out.Close();

        // Decrypt Np stuff and overwrite.
        if (entry.id == 0x400 || entry.id == 0x401 || entry.id == 0x402 ||
            entry.id == 0x403) { // somehow 0x401 is not decrypting
            decNp.resize(entry.size);
            if (!file.Seek(entry.offset)) {
                failreason = "Failed to seek to PKG entry offset";
                return false;
            }

            std::vector<u8> data;
            data.resize(entry.size);
            file.ReadRaw<u8>(data.data(), entry.size);

            std::span<u8> cipherNp(data.data(), entry.size);
            std::array<u8, 64> concatenated_ivkey_dk3_;
            std::memcpy(concatenated_ivkey_dk3_.data(), &entry, sizeof(entry));
            std::memcpy(concatenated_ivkey_dk3_.data() + sizeof(entry), dk3_.data(), sizeof(dk3_));
            PKG::crypto.ivKeyHASH256(concatenated_ivkey_dk3_, ivKey);
            PKG::crypto.aesCbcCfb128DecryptEntry(ivKey, cipherNp, decNp);

            Common::FS::IOFile out(extract_path / "sce_sys" / name,
                                   Common::FS::FileAccessMode::Write);
            out.Write(decNp);
            out.Close();
        }

        file.Seek(currentPos);
    }

    // Read the seed
    std::array<u8, 16> seed;
    if (!file.Seek(pkgheader.pfs_image_offset + 0x370)) {
        failreason = "Failed to seek to PFS image offset";
        return false;
    }
    file.Read(seed);

    // Get data and tweak keys.
    PKG::crypto.PfsGenCryptoKey(ekpfsKey, seed, dataKey, tweakKey);
    const u32 length = pkgheader.pfs_cache_size * 0x2;

    // Declare pfsc outside the block so it persists for iteration
    int num_blocks = 0;
    std::vector<u8> pfsc;
    
    // Debug: Print length immediately
    {
        PKGProgress len_prog{};
        len_prog.stage = PKGProgress::Stage::ParsingPFS;
        len_prog.message = "PFS cache length: " + std::to_string(length) + " bytes (pfs_cache_size=" + std::to_string(pkgheader.pfs_cache_size) + ")";
        if (progress_cb_) {
            progress_cb_(len_prog);
        }
    }
    
    if (length == 0) {
        failreason = "PFS cache size is 0 - cannot extract";
        prog.stage = PKGProgress::Stage::Error;
        prog.message = failreason;
        reportProgress(prog);
        return false;
    }
    
    if (length != 0) {
        // Read the full PFS cache region
        std::vector<u8> pfs_encrypted(length);
        std::vector<u8> pfs_decrypted(length);
        
        file.Seek(pkgheader.pfs_image_offset);
        file.Read(pfs_encrypted);
        file.Close();
        
        // Decrypt the entire PFS cache region once
        PKG::crypto.decryptPFS(dataKey, tweakKey, pfs_encrypted, pfs_decrypted, 0);
        
        // Find PFSC offset in the decrypted data
        pfsc_offset = GetPFSCOffset(pfs_decrypted);
        
        if (pfsc_offset == static_cast<u32>(-1)) {
            failreason = "Could not find PFSC in PFS image";
            prog.stage = PKGProgress::Stage::Error;
            prog.message = failreason;
            reportProgress(prog);
            return false;
        }
        
        // Copy the PFSC structure into memory for parsing
        if (pfsc_offset >= pfs_decrypted.size()) {
            failreason = "PFSC offset is beyond decrypted data size";
            prog.stage = PKGProgress::Stage::Error;
            prog.message = failreason;
            reportProgress(prog);
            return false;
        }
        
        pfsc.resize(length - pfsc_offset);
        std::memcpy(pfsc.data(), pfs_decrypted.data() + pfsc_offset, length - pfsc_offset);
        
        // Read PFSC header to get block information
        PFSCHdr pfsChdr;
        if (sizeof(pfsChdr) > pfsc.size()) {
            failreason = "PFSC buffer too small for header";
            prog.stage = PKGProgress::Stage::Error;
            prog.message = failreason;
            reportProgress(prog);
            return false;
        }
        std::memcpy(&pfsChdr, pfsc.data(), sizeof(pfsChdr));
        
        num_blocks = static_cast<int>(pfsChdr.data_length / pfsChdr.block_sz2);
        
        // Build sector map from PFSC header
        sectorMap.resize(num_blocks + 1);
        for (int i = 0; i < num_blocks + 1; i++) {
            u32 map_offset = pfsChdr.block_offsets + i * 8;
            if (map_offset + 8 > pfsc.size()) {
                failreason = "Sector map offset exceeds PFSC size";
                prog.stage = PKGProgress::Stage::Error;
                prog.message = failreason;
                reportProgress(prog);
                return false;
            }
            std::memcpy(&sectorMap[i], pfsc.data() + map_offset, 8);
        }
        
        // Report parsing parameters (force without throttling)
        PKGProgress parse_init{};
        parse_init.stage = PKGProgress::Stage::ParsingPFS;
        parse_init.message = "PFS blocks: " + std::to_string(num_blocks) + ", PFSC offset: 0x" + 
                           std::to_string(pfsc_offset) + ", PFSC size: " + std::to_string(pfsc.size());
        // Force reportProgress to ensure this message gets through
        if (progress_cb_) {
            progress_cb_(parse_init);
        }
    }

    u32 ent_size = 0;
    u32 ndinode = 0;
    int ndinode_counter = 0;
    bool dinode_reached = false;
    bool uroot_reached = false;
    std::vector<char> compressedData;
    std::vector<char> decompressedData(0x10000);
    
    // Safety: Reserve reasonable capacity to avoid reallocations
    fsTable.reserve(50000);
    iNodeBuf.reserve(50000);

    // Get iNodes and Dirents - data is already decrypted in pfsc buffer
    prog.stage = PKGProgress::Stage::ParsingPFS;
    prog.message = "Parsing PFS";
    reportProgress(prog);
    
    auto last_progress_time = std::chrono::steady_clock::now();
    
    // Get iNodes and Dirents - iterate through all blocks
    for (int i = 0; i < num_blocks; i++) {
        // Report parsing progress every 100ms
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_progress_time).count() >= 100) {
            PKGProgress parse_prog{};
            parse_prog.stage = PKGProgress::Stage::ParsingPFS;
            parse_prog.message = "Parsing PFS structure";
            parse_prog.percent = (static_cast<double>(i) / num_blocks) * 50.0; // 0-50% for parsing
            reportProgress(parse_prog);
            last_progress_time = now;
        }
        
        const u64 sectorOffset = sectorMap[i];
        const u64 sectorSize = sectorMap[i + 1] - sectorOffset;

        compressedData.resize(sectorSize);
        std::memcpy(compressedData.data(), pfsc.data() + sectorOffset, sectorSize);

        if (sectorSize == 0x10000) { // Uncompressed data
            std::memcpy(decompressedData.data(), compressedData.data(), 0x10000);
        } else if (sectorSize < 0x10000) { // Compressed data
            if (!DecompressPFSC(compressedData, decompressedData)) {
                continue; // Skip this block if decompression failed
            }
        } else {
            continue; // Skip blocks with invalid size
        }

        if (i == 0) {
            std::memcpy(&ndinode, decompressedData.data() + 0x30, 4); // number of folders and files
            
            // Safety check: Reject obviously corrupted values
            if (ndinode > 500000) {
                failreason = "PFS appears corrupted: too many inodes";
                return false;
            }
            
            // Report expected file count
            PKGProgress inode_prog{};
            inode_prog.stage = PKGProgress::Stage::ParsingPFS;
            inode_prog.message = "Expecting " + std::to_string(ndinode) + " entries";
            inode_prog.percent = 1.0;
            reportProgress(inode_prog);
        }

        int occupied_blocks =
            (ndinode * 0xA8) / 0x10000; // how many blocks(0x10000) are taken by iNodes.
        if (((ndinode * 0xA8) % 0x10000) != 0)
            occupied_blocks += 1;

        if (i >= 1 && i <= occupied_blocks) { // Get all iNodes, gives type, file size and location.
            for (int p = 0; p < 0x10000; p += 0xA8) {
                // Bounds check before accessing decompressed data
                if (p + sizeof(Inode) > decompressedData.size()) {
                    break; // Prevent buffer overflow
                }
                
                Inode node;
                std::memcpy(&node, &decompressedData[p], sizeof(node));
                if (node.Mode == 0) {
                    break;
                }
                
                iNodeBuf.push_back(node);
            }
        }

        // let's deal with the root/uroot entries here.
        // Sometimes it's more than 2 entries (Tomb Raider Remastered)
        const std::string_view flat_path_table(&decompressedData[0x10], 15);
        if (flat_path_table == "flat_path_table") {
            uroot_reached = true;
        }

        if (uroot_reached) {
            for (int i = 0; i < 0x10000; i += ent_size) {
                Dirent dirent;
                std::memcpy(&dirent, &decompressedData[i], sizeof(dirent));
                ent_size = dirent.entsize;
                if (dirent.ino != 0) {
                    ndinode_counter++;
                } else {
                    // Set the the folder according to the current inode.
                    // Can be 2 or more (rarely)
                    // Use the user provided extraction root as-is instead of
                    // rewriting the path based on title id. The previous logic
                    // could create an extra nested title id directory or place
                    // files beside the user-selected folder unexpectedly.
                    extractPaths[ndinode_counter] = extract_path;
                    uroot_reached = false;
                    break;
                }
            }
        }

        const char dot = decompressedData[0x10];
        const std::string_view dotdot(&decompressedData[0x28], 2);
        if (dot == '.' && dotdot == "..") {
            dinode_reached = true;
        }

        // Get folder and file names.
        bool end_reached = false;
        if (dinode_reached) {
            for (int j = 0; j < 0x10000; j += ent_size) { // Skip the first parent and child.
                Dirent dirent;
                std::memcpy(&dirent, &decompressedData[j], sizeof(dirent));

                // Stop here and continue the main loop
                if (dirent.ino == 0) {
                    break;
                }

                ent_size = dirent.entsize;
                auto& table = fsTable.emplace_back();
                table.name = std::string(dirent.name, dirent.namelen);
                table.inode = dirent.ino;
                table.type = dirent.type;

                // Handle current directory marker - updates current_dir
                if (table.type == PFS_CURRENT_DIR) {
                    current_dir = extractPaths[table.inode];
                }
                // Always update the path for this inode
                extractPaths[table.inode] = current_dir / std::filesystem::path(table.name);

                if (table.type == PFS_FILE || table.type == PFS_DIR) {
                    if (table.type == PFS_DIR) { // Create dir (and parents) safely.
                        std::filesystem::create_directories(extractPaths[table.inode]);
                    }
                    ndinode_counter++;
                    if ((ndinode_counter + 1) == ndinode) // 1 for the image itself (root).
                        end_reached = true;
                }
            }
            if (end_reached) {
                break;
            }
        }
    }
    
    // Report what we found
    PKGProgress found_prog{};
    found_prog.stage = PKGProgress::Stage::ParsingPFS;
    found_prog.message = "Found " + std::to_string(fsTable.size()) + " entries";
    found_prog.percent = 50.0;
    reportProgress(found_prog);
    
    // Compute totals for extraction progress
    extract_files_total_ = 0;
    extract_bytes_total_ = 0;
    for (const auto& ent : fsTable) {
        if (ent.type == PFS_FILE) {
            extract_files_total_++;
            int inode = ent.inode;
            if (inode >= 0 && inode < static_cast<int>(iNodeBuf.size())) {
                extract_bytes_total_ += static_cast<u64>(iNodeBuf[inode].Size);
            }
        }
    }
    extract_files_done_ = 0;
    extract_bytes_done_ = 0;

    // Report parsing complete
    prog.stage = PKGProgress::Stage::ParsingPFS;
    prog.message = "PFS parsing complete";
    prog.files_total = extract_files_total_;
    prog.bytes_total = extract_bytes_total_;
    prog.percent = 50.0;
    reportProgress(prog);

    return true;
}

void PKG::ExtractFiles(const int index) {
    // Bounds check for index
    if (index < 0 || index >= static_cast<int>(fsTable.size())) {
        return; // Invalid index
    }
    
    int inode_number = fsTable[index].inode;
    int inode_type = fsTable[index].type;
    std::string inode_name = fsTable[index].name;

    if (inode_type == PFS_FILE) {
        int sector_loc = iNodeBuf[inode_number].loc;
        int nblocks = iNodeBuf[inode_number].Blocks;
        int bsize = iNodeBuf[inode_number].Size;

        Common::FS::IOFile inflated;
        inflated.Open(extractPaths[inode_number], Common::FS::FileAccessMode::Write);

        Common::FS::IOFile pkgFile; // Open the file for each iteration to avoid conflict.
        pkgFile.Open(pkgpath, Common::FS::FileAccessMode::Read);

        int size_decompressed = 0;
        std::vector<char> compressedData;
        std::vector<char> decompressedData(0x10000);

        u64 pfsc_buf_size = 0x11000; // extra 0x1000
        std::vector<u8> pfsc(pfsc_buf_size);
        std::vector<u8> pfs_decrypted(pfsc_buf_size);
        
        // Throttle progress updates to avoid UI freeze
        auto last_block_progress = std::chrono::steady_clock::now();

        for (int j = 0; j < nblocks; j++) {
            u64 sectorOffset =
                sectorMap[sector_loc + j]; // offset into PFSC_image and not pfs_image.
            u64 sectorSize = sectorMap[sector_loc + j + 1] -
                             sectorOffset; // indicates if data is compressed or not.
            u64 fileOffset = (pkgheader.pfs_image_offset + pfsc_offset + sectorOffset);
            u64 currentSector1 =
                (pfsc_offset + sectorOffset) / 0x1000; // block size is 0x1000 for xts decryption.

            int sectorOffsetMask = (sectorOffset + pfsc_offset) & 0xFFFFF000;
            int previousData = (sectorOffset + pfsc_offset) - sectorOffsetMask;

            pkgFile.Seek(fileOffset - previousData);
            pkgFile.Read(pfsc);

            PKG::crypto.decryptPFS(dataKey, tweakKey, pfsc, pfs_decrypted, currentSector1);

            compressedData.resize(sectorSize);
            std::memcpy(compressedData.data(), pfs_decrypted.data() + previousData, sectorSize);

            if (sectorSize == 0x10000) // Uncompressed data
                std::memcpy(decompressedData.data(), compressedData.data(), 0x10000);
            else if (sectorSize < 0x10000) // Compressed data
                DecompressPFSC(compressedData, decompressedData);

            size_decompressed += 0x10000;

            // Update per-block progress using decompressed chunk size (throttled to 100ms)
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_block_progress).count() >= 100) {
                u64 file_size = static_cast<u64>(bsize);
                u64 new_done = std::min<u64>(static_cast<u64>(size_decompressed), file_size);
                // Update global counters if available
                if (extract_bytes_total_ > 0) {
                    PKGProgress blk{};
                    blk.stage = PKGProgress::Stage::Extracting;
                    blk.current_file = inode_name;
                    blk.files_total = extract_files_total_;
                    blk.files_done = extract_files_done_;
                    blk.bytes_total = extract_bytes_total_;
                    blk.bytes_done = extract_bytes_done_ + new_done;
                    blk.percent = std::min(100.0, 100.0 * static_cast<double>(blk.bytes_done) / static_cast<double>(blk.bytes_total));
                    blk.message = "Extracting " + inode_name;
                    reportProgress(blk);
                    last_block_progress = now;
                }
            }

            if (j < nblocks - 1) {
                inflated.WriteRaw<u8>(decompressedData.data(), decompressedData.size());
            } else {
                // This is to remove the zeros at the end of the file.
                const u32 write_size = decompressedData.size() - (size_decompressed - bsize);
                inflated.WriteRaw<u8>(decompressedData.data(), write_size);
            }
        }
        pkgFile.Close();
        inflated.Close();
    }
    
    // Update global extraction counters after file completes
    extract_files_done_++;
    if (inode_number >= 0 && inode_number < static_cast<int>(iNodeBuf.size())) {
        extract_bytes_done_ += static_cast<u64>(iNodeBuf[inode_number].Size);
    }
    
    // Report file completion
    PKGProgress file_done{};
    file_done.stage = PKGProgress::Stage::Extracting;
    file_done.current_file = inode_name;
    file_done.files_total = extract_files_total_;
    file_done.files_done = extract_files_done_;
    file_done.bytes_total = extract_bytes_total_;
    file_done.bytes_done = extract_bytes_done_;
    if (extract_bytes_total_ > 0) {
        file_done.percent = std::min(100.0, 100.0 * static_cast<double>(file_done.bytes_done) / static_cast<double>(file_done.bytes_total));
    } else if (extract_files_total_ > 0) {
        file_done.percent = std::min(100.0, 100.0 * static_cast<double>(file_done.files_done) / static_cast<double>(file_done.files_total));
    }
    file_done.message = "Extracted " + inode_name;
    reportProgress(file_done);
} // End of ExtractFiles function
