# PKG Extraction Bug Fixes - October 6, 2025

## Issue: GUI Only Extracted Metadata, Not Game Files

### Problem
When extracting PKG files through the GUI, only directories and `sce_sys` metadata files (28 files) were created, but actual game files (`eboot.bin`, `.plt`, etc.) were not extracted. CLI extraction worked correctly.

**Example:**
- Expected: 331 files including `eboot.bin` (29MB), game assets
- Actual: 28 files in `sce_sys/` directory only
- Missing: All game executable and data files

### Root Cause
The GUI called `PKG::Extract()` but never called `PKG::ExtractFiles(index)` for each file!

According to the reference implementation (`ps4-pkg-tools`):
```cpp
// After Extract() parses PFS:
for (int idx : file_indices) {
    pkg.ExtractFiles(idx);  // MUST call for each file!
}
```

Our GUI code only called:
```cpp
bool extractSuccess = pkg.Extract(pkgPathFs, extractPath, failReason);
// ❌ MISSING: Loop to call ExtractFiles() for each file
```

### Solution
Modified `/home/jon/ShadPs4Manager/src/gui/working_gui.cpp` line ~469:

**Before:**
```cpp
bool extractFailed = !pkg.Extract(pkgPathFs, std::filesystem::path(actualExtractionPath.toStdString()), failReason);

// Handle update merging if this was an update
if (extractFailed && isUpdate) {  // ❌ Wrong logic
```

**After:**
```cpp
bool extractSuccess = pkg.Extract(pkgPathFs, std::filesystem::path(actualExtractionPath.toStdString()), failReason);

// ✅ Extract all individual files
if (extractSuccess) {
    uint32_t totalFiles = pkg.GetNumberOfFiles();
    for (uint32_t i = 0; i < totalFiles; ++i) {
        try {
            pkg.ExtractFiles(static_cast<int>(i));
        } catch (const std::exception& e) {
            failReason = std::string("Failed to extract file index ") + std::to_string(i) + ": " + e.what();
            extractSuccess = false;
            break;
        } catch (...) {
            failReason = std::string("Unknown error extracting file index ") + std::to_string(i);
            extractSuccess = false;
            break;
        }
        
        if (!dialogPtr) {
            extractSuccess = false;
            failReason = "Extraction cancelled by user";
            break;
        }
    }
}

// Handle update merging if this was an update
if (extractSuccess && isUpdate) {  // ✅ Fixed logic
```

### Changes Summary
1. **Renamed variable**: `extractFailed` → `extractSuccess` for clarity
2. **Fixed logic**: Changed `if (extractFailed && isUpdate)` → `if (extractSuccess && isUpdate)`
3. **Added extraction loop**: Call `ExtractFiles(i)` for all `i` from 0 to `GetNumberOfFiles()-1`
4. **Added error handling**: Try-catch for each file extraction
5. **Added cancellation check**: Verify `dialogPtr` is valid (user didn't close dialog)

---

## Issue: Double-Free Crash on Library Refresh

### Problem
After successfully extracting a PKG and clicking "Yes" to refresh the library, the application crashed with:
```
double free or corruption (!prev)
Aborted (core dumped)
```

### Root Cause
In `GameLibrary::clearGames()`, widgets were scheduled for deletion with `deleteLater()` but their pointers remained in the list. When `loadGames()` created new cards and called `arrangeGameCards()`, Qt tried to manage the old widgets that were pending deletion, causing a double-free.

**Problematic code** in `/home/jon/ShadPs4Manager/src/gui/game_library.cpp`:
```cpp
void GameLibrary::clearGames() {
    for (GameCard* card : gameCards) {
        cardsLayout->removeWidget(card);
        card->deleteLater();  // ❌ Schedules deletion for later
    }
    gameCards.clear();  // Clears list but widgets still pending deletion
    games.clear();
}
```

When `arrangeGameCards()` was called immediately after:
```cpp
void GameLibrary::arrangeGameCards() {
    while ((item = cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->setParent(nullptr);  // Might reference old widgets
        }
        delete item;
    }
    // Add new cards
    for (int i = 0; i < gameCards.size(); ++i) {
        cardsLayout->addWidget(gameCards[i], row, col);
    }
}
```

The sequence caused:
1. Old widgets scheduled for `deleteLater()` but not yet deleted
2. New widgets created and added to layout
3. Layout operations might reference both old and new widgets
4. Qt event loop processes deletions → double-free or use-after-free

### Solution
Delete widgets immediately instead of using `deleteLater()`:

**After:**
```cpp
void GameLibrary::clearGames() {
    for (GameCard* card : gameCards) {
        cardsLayout->removeWidget(card);
        card->setParent(nullptr);
        delete card;  // ✅ Delete immediately, not later
    }
    gameCards.clear();
    games.clear();
}
```

This ensures:
- Widgets are fully deleted before new ones are created
- No pending deletions when `arrangeGameCards()` runs
- Clean slate for the new library refresh

---

## Verification

### Test Case: JP1091 PKG (676MB)
**Before Fix:**
- Files extracted: 28 (only `sce_sys/`)
- Missing: `eboot.bin`, game assets
- Total size: ~5MB

**After Fix:**
- Files extracted: 331 (all files including game data)
- Includes: `eboot.bin` (29MB), `sce_discmap.plt` (3.3MB), game assets
- Total size: 729MB
- Library refresh: No crash ✅

### Test Case: DRIVECLUB PKG (15GB)
- Successfully extracted 72 files, 16GB total
- Library refresh after extraction: No crash ✅
- Update merge functionality: Working correctly ✅

---

## Lessons Learned

1. **Read reference implementations carefully**: The ps4-pkg-tools clearly showed the two-step process (Extract → loop ExtractFiles)

2. **Qt memory management**: Be careful mixing `deleteLater()` with immediate widget creation - prefer immediate deletion in synchronous refresh operations

3. **Test extraction completeness**: Don't just check for "success" - verify actual file count and sizes

4. **Variable naming matters**: `extractFailed` vs `extractSuccess` - choosing the right polarity prevents logic errors

---

## Related Files Modified
- `/home/jon/ShadPs4Manager/src/gui/working_gui.cpp` - Added ExtractFiles loop
- `/home/jon/ShadPs4Manager/src/gui/game_library.cpp` - Changed deleteLater to immediate delete

## Build Version
- Date: October 6, 2025
- Commit: Post-extraction bug fix
- Executables: 
  - `shadps4-manager-gui` (3.5M)
  - `shadps4-pkg-extractor` (2.9M)
