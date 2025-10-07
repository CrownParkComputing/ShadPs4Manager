# GUI Progress Display - Visual Reference

## Progress Dialog Layout

When extracting a PKG file, the GUI will show a progress dialog with the following information:

```
┌─────────────────────────────────────────────────────┐
│  ShadPs4 Manager - PKG Extraction                   │
├─────────────────────────────────────────────────────┤
│                                                      │
│  Extracting files...                                │
│                                                      │
│  File: game007.dat                                  │
│  Progress: 14 / 60 files (53%)                      │
│  Data: 8.30 GB / 15.64 GB                          │
│                                                      │
│  [████████████████████░░░░░░░░░░░░░]  53%          │
│                                                      │
│                            [Cancel]                  │
└─────────────────────────────────────────────────────┘
```

## Stage Messages

The progress dialog shows different messages based on the extraction stage:

### 1. Opening Stage
```
Opening PKG file...
```

### 2. Reading Metadata Stage
```
Reading PKG metadata...
```

### 3. Parsing PFS Stage
```
Parsing PFS structure...

Expecting 67 entries
```

### 4. Extracting Stage (Main Phase)
```
Extracting files...

File: game007.dat
Progress: 14 / 60 files (53%)
Data: 8.30 GB / 15.64 GB
```

### 5. Done Stage
```
Extraction complete!
```

### 6. Error Stage (if errors occur)
```
Error during extraction

[Error message here]
```

## Real-Time Updates

- **Progress bar:** Updates with percentage (0-100%)
- **File name:** Shows current file being extracted
- **File count:** Shows files_done / files_total
- **Byte count:** Shows human-readable data transferred (KB, MB, GB)
- **Update frequency:** Throttled to 100ms to avoid UI freeze

## Example Timeline (DRIVECLUB 15GB)

```
[00:00] Opening PKG file...                        0%
[00:01] Reading PKG metadata...                    1%
[00:02] Parsing PFS structure...                   5%
        Expecting 67 entries
        Found 72 entries
[00:03] Extracting files...                       10%
        File: eboot.bin
        Progress: 1 / 60 files (2%)
        Data: 18.0 MB / 15.64 GB
[00:15] Extracting files...                       30%
        File: game003.dat
        Progress: 6 / 60 files (10%)
        Data: 4.89 GB / 15.64 GB
[00:45] Extracting files...                       70%
        File: game010.dat
        Progress: 18 / 60 files (30%)
        Data: 11.78 GB / 15.64 GB
[01:17] Extraction complete!                     100%
```

## Features

✅ **Real-time progress:** Updates every 100ms during extraction
✅ **Human-readable sizes:** Automatic formatting (B, KB, MB, GB, TB)
✅ **Stage awareness:** Different messages for each extraction phase
✅ **Cancel support:** User can cancel extraction at any time
✅ **Non-blocking UI:** Uses QApplication::processEvents() to keep UI responsive
✅ **Error handling:** Graceful error display with detailed messages

## How to Test

1. Launch GUI:
   ```bash
   /home/jon/ShadPs4Manager/build/bin/shadps4-manager-gui
   ```

2. Open Downloads tab

3. Select any PKG file (e.g., JP1091 or DRIVECLUB)

4. Click "Extract" button

5. Watch the progress dialog show:
   - Stage messages
   - Current file name
   - File count progress
   - Data transfer progress
   - Progress bar animation

## Success Confirmation

After extraction completes, you'll see a success dialog:

```
┌─────────────────────────────────────────────────────┐
│  Extraction Complete                                │
├─────────────────────────────────────────────────────┤
│                                                      │
│  PKG file extracted successfully to:                │
│  /path/to/output                                    │
│                                                      │
│  Title ID: CUSA00093                                │
│  Files extracted: 72                                │
│  PKG Size: 14.66 GB                                 │
│                                                      │
│                              [OK]                    │
└─────────────────────────────────────────────────────┘
```

Then optionally refresh the game library to show the newly extracted game.
