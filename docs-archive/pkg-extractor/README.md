# PKG Extractor CLI

Command-line tool for extracting PS4 PKG files.

## Overview

The PKG Extractor is a lightweight command-line tool for extracting PS4 package files without requiring a GUI. Perfect for:
- Automated scripts
- Batch processing
- Server environments
- CI/CD pipelines
- Headless systems

**Size**: ~3.0 MB  
**License**: GPL-3.0

## Features

- Fast PKG extraction
- Support for base games, updates, and DLC
- Verbose output mode
- Automatic directory creation
- Progress reporting
- Return codes for scripting
- No GUI dependencies

## System Requirements

### Minimum Requirements
- **OS**: Linux, Windows, macOS
- **CPU**: Dual-core 1.5 GHz
- **RAM**: 512 MB
- **Disk**: 10 MB + space for extracted files

### Dependencies
- Crypto++ (statically linked)
- zlib (statically linked)
- No Qt or GUI libraries required

## Installation

### Linux

**From Binary:**
```bash
# Extract archive
tar -xzf shadps4-pkg-extractor-linux-x64-v1.0.1.tar.gz

# Make executable
chmod +x shadps4-pkg-extractor

# Test
./shadps4-pkg-extractor --help
```

**System-wide installation:**
```bash
sudo cp shadps4-pkg-extractor /usr/local/bin/
sudo chmod +x /usr/local/bin/shadps4-pkg-extractor

# Run from anywhere
shadps4-pkg-extractor --help
```

### Windows

1. Extract ZIP file
2. Open Command Prompt or PowerShell
3. Navigate to extraction directory
4. Run: `shadps4-pkg-extractor.exe --help`

**Add to PATH (optional):**
```powershell
# PowerShell (as Administrator)
$env:Path += ";C:\path\to\extractor"
[Environment]::SetEnvironmentVariable("Path", $env:Path, [EnvironmentVariableTarget]::Machine)
```

### macOS

```bash
# Extract archive
tar -xzf shadps4-pkg-extractor-macos-universal-v1.0.1.tar.gz

# Make executable
chmod +x shadps4-pkg-extractor

# Move to PATH
sudo mv shadps4-pkg-extractor /usr/local/bin/

# Run
shadps4-pkg-extractor --help
```

## Usage

### Basic Syntax

```bash
shadps4-pkg-extractor <input.pkg> <output_directory> [options]
```

### Examples

**Extract a game:**
```bash
shadps4-pkg-extractor game.pkg /output/games/
```

**Verbose output:**
```bash
shadps4-pkg-extractor game.pkg /output/games/ --verbose
```

**Extract to current directory:**
```bash
shadps4-pkg-extractor game.pkg ./
```

**Extract multiple files:**
```bash
for pkg in *.pkg; do
    shadps4-pkg-extractor "$pkg" "/output/$(basename "$pkg" .pkg)"
done
```

### Options

| Option | Description |
|--------|-------------|
| `--verbose`, `-v` | Enable verbose output |
| `--help`, `-h` | Show help message |
| `--version` | Show version information |
| `--quiet`, `-q` | Suppress non-error output |

### Return Codes

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | Invalid arguments |
| `2` | File not found |
| `3` | Extraction failed |
| `4` | Insufficient disk space |
| `5` | Permission denied |

Check return code in scripts:
```bash
shadps4-pkg-extractor game.pkg /output/
if [ $? -eq 0 ]; then
    echo "Extraction successful"
else
    echo "Extraction failed with code $?"
fi
```

## Output Format

### Normal Mode

```
Extracting: game.pkg
Output: /output/games/CUSA12345
Progress: [=========>        ] 45%
Size: 25.3 GB / 56.0 GB
Time: 00:05:23
Extraction complete!
```

### Verbose Mode

```
Opening PKG file: game.pkg
PKG Type: Base Game
Title ID: CUSA12345
Title: Example Game
Version: 01.00
Content ID: UP1234-CUSA12345_00-EXAMPLEGAME00000
PKG Size: 56.0 GB
Encrypted: Yes
Creating output directory: /output/games/CUSA12345
Extracting file: param.sfo (2048 bytes)
Extracting file: icon0.png (256 KB)
Extracting file: pic0.png (1.2 MB)
Extracting file: app/EBOOT.BIN (45 MB)
...
Extraction complete in 00:08:45
Total files: 12,543
Total size: 56.0 GB
```

### Quiet Mode

No output unless error occurs.

## Batch Processing

### Extract All PKGs in Directory

```bash
#!/bin/bash
INPUT_DIR="/path/to/pkgs"
OUTPUT_DIR="/path/to/output"

for pkg in "$INPUT_DIR"/*.pkg; do
    echo "Processing: $(basename "$pkg")"
    shadps4-pkg-extractor "$pkg" "$OUTPUT_DIR" --verbose
    
    if [ $? -ne 0 ]; then
        echo "Failed: $(basename "$pkg")" >> errors.log
    fi
done

echo "Batch extraction complete!"
```

### Parallel Processing

```bash
#!/bin/bash
INPUT_DIR="/path/to/pkgs"
OUTPUT_DIR="/path/to/output"
MAX_JOBS=4

export -f extract_pkg

extract_pkg() {
    local pkg=$1
    local output=$2
    shadps4-pkg-extractor "$pkg" "$output" --quiet
}

find "$INPUT_DIR" -name "*.pkg" | \
    xargs -P "$MAX_JOBS" -I {} \
    bash -c 'extract_pkg "$@"' _ {} "$OUTPUT_DIR"
```

### Filter by Type

```bash
#!/bin/bash
# Extract only base games (not updates or DLC)

for pkg in *.pkg; do
    # Check if base game (basic heuristic)
    if [[ ! "$pkg" =~ (patch|update|dlc) ]]; then
        echo "Extracting base game: $pkg"
        shadps4-pkg-extractor "$pkg" "/output/base-games/"
    fi
done
```

## Automation Examples

### Systemd Service (Linux)

Create `/etc/systemd/system/pkg-extractor.service`:

```ini
[Unit]
Description=PKG Extractor Watch Service
After=network.target

[Service]
Type=simple
User=shadps4
WorkingDirectory=/srv/shadps4
ExecStart=/usr/local/bin/pkg-watch.sh
Restart=always

[Install]
WantedBy=multi-user.target
```

Watch script (`/usr/local/bin/pkg-watch.sh`):

```bash
#!/bin/bash
INPUT="/srv/shadps4/incoming"
OUTPUT="/srv/shadps4/games"

inotifywait -m -e create "$INPUT" --format '%f' | while read file; do
    if [[ "$file" == *.pkg ]]; then
        echo "New PKG detected: $file"
        shadps4-pkg-extractor "$INPUT/$file" "$OUTPUT/" --verbose
        
        if [ $? -eq 0 ]; then
            echo "Moving processed PKG to archive"
            mv "$INPUT/$file" "$INPUT/processed/"
        fi
    fi
done
```

### Cron Job

```bash
# Add to crontab: crontab -e

# Extract PKGs daily at 2 AM
0 2 * * * /usr/local/bin/extract-daily.sh >> /var/log/pkg-extractor.log 2>&1
```

Extract script:

```bash
#!/bin/bash
INPUT="/srv/downloads"
OUTPUT="/srv/games"
DATE=$(date +%Y-%m-%d)

echo "[$DATE] Starting extraction..."

for pkg in "$INPUT"/*.pkg; do
    if [ -f "$pkg" ]; then
        shadps4-pkg-extractor "$pkg" "$OUTPUT/" --verbose
    fi
done

echo "[$DATE] Extraction complete"
```

### Docker Integration

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY shadps4-pkg-extractor /usr/local/bin/
RUN chmod +x /usr/local/bin/shadps4-pkg-extractor

WORKDIR /data

ENTRYPOINT ["shadps4-pkg-extractor"]
CMD ["--help"]
```

Run:
```bash
docker build -t pkg-extractor .
docker run -v /host/pkgs:/input -v /host/output:/output \
    pkg-extractor /input/game.pkg /output/
```

## Integration with Other Tools

### Python Script

```python
#!/usr/bin/env python3
import subprocess
import os
import sys

def extract_pkg(pkg_file, output_dir):
    """Extract a PKG file using the CLI tool"""
    cmd = ['shadps4-pkg-extractor', pkg_file, output_dir, '--verbose']
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"Success: {pkg_file}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error extracting {pkg_file}: {e}", file=sys.stderr)
        return False

# Usage
extract_pkg('/path/to/game.pkg', '/output/')
```

### Node.js Script

```javascript
const { spawn } = require('child_process');
const path = require('path');

function extractPkg(pkgFile, outputDir) {
    return new Promise((resolve, reject) => {
        const extractor = spawn('shadps4-pkg-extractor', 
            [pkgFile, outputDir, '--verbose']);
        
        extractor.stdout.on('data', (data) => {
            console.log(`${data}`);
        });
        
        extractor.stderr.on('data', (data) => {
            console.error(`Error: ${data}`);
        });
        
        extractor.on('close', (code) => {
            if (code === 0) {
                resolve();
            } else {
                reject(new Error(`Extraction failed with code ${code}`));
            }
        });
    });
}

// Usage
extractPkg('/path/to/game.pkg', '/output/')
    .then(() => console.log('Extraction complete'))
    .catch(err => console.error(err));
```

## Troubleshooting

### "Permission Denied"

```bash
# Linux/macOS
chmod +x shadps4-pkg-extractor

# Or run with sudo (not recommended)
sudo shadps4-pkg-extractor game.pkg /output/
```

### "File Not Found"

```bash
# Check file exists
ls -lh game.pkg

# Use absolute path
shadps4-pkg-extractor /full/path/to/game.pkg /output/
```

### "Insufficient Disk Space"

```bash
# Check available space
df -h /output/directory

# Clear space or use different output directory
shadps4-pkg-extractor game.pkg /different/path/
```

### "Invalid PKG File"

```bash
# Verify PKG integrity
sha256sum game.pkg

# Check file size
ls -lh game.pkg

# Try verbose mode for more info
shadps4-pkg-extractor game.pkg /output/ --verbose
```

### Extraction Hangs

```bash
# Kill process
pkill shadps4-pkg-extractor

# Check system resources
top
df -h

# Try with smaller PKG first
```

## Performance Tips

### Optimize Extraction Speed

1. **Use SSD**: Extract to solid-state drive
2. **Sufficient RAM**: 4GB+ recommended for large PKGs
3. **Close other apps**: Free system resources
4. **Local storage**: Avoid network drives

### Benchmarks

Typical extraction times (SSD, 8-core CPU):

| PKG Size | Time (Normal) | Time (Verbose) |
|----------|---------------|----------------|
| 1 GB     | ~15 seconds   | ~20 seconds    |
| 10 GB    | ~2 minutes    | ~2.5 minutes   |
| 50 GB    | ~8 minutes    | ~10 minutes    |
| 100 GB   | ~15 minutes   | ~18 minutes    |

## Building from Source

See [BUILD.md](BUILD.md) in this directory for build instructions.

## Comparison with GUI

| Feature | CLI | GUI |
|---------|-----|-----|
| **Size** | 3.0 MB | 3.8 MB |
| **Dependencies** | None | Qt6 |
| **Headless** | ✅ Yes | ❌ No |
| **Scriptable** | ✅ Yes | ⚠️ Limited |
| **Progress UI** | Text | Visual |
| **Batch** | ✅ Excellent | ⚠️ Manual |
| **IGDB** | ❌ No | ✅ Yes |
| **License** | Not enforced | 10-item trial |

## Support

- **Issues**: [GitHub Issues](https://github.com/CrownParkComputing/ShadPs4Manager/issues)
- **Documentation**: [Main Documentation](../../README.md)
- **Build Guide**: [BUILD.md](BUILD.md)

---

**Version**: 1.0.1  
**License**: GPL-3.0  
**Website**: https://crownparkcomputing.github.io/ShadPs4Manager/
