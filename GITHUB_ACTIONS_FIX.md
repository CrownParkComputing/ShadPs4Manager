# GitHub Actions Build Fixes

## Problem Summary
The GitHub Actions workflows were failing on all platforms (Linux, Windows, macOS) with Python-related errors:
- `Error: The process '/Users/runner/hostedtoolcache/Python/3.11.9/arm64/bin/python3' failed with exit code 1` (macOS)
- `Error: The process 'C:\hostedtoolcache\windows\Python\3.11.9\x64\python.exe' failed with exit code 1` (Windows)
- `Error: The process '/opt/hostedtoolcache/Python/3.11.13/x64/bin/python3' failed with exit code 1` (Linux)

## Root Cause
The `jurplel/install-qt-action@v3` action uses Python and `aqtinstall` internally to download and install Qt. The failures were caused by:

1. **Missing Python setup**: The workflows didn't explicitly set up Python before the Qt installation action
2. **Inconsistent Qt parameters**: Missing or incorrect parameters like `host` and `arch` specifications
3. **Python environment issues**: The action's internal Python setup was failing or encountering compatibility issues

## Changes Made

### 1. Added Explicit Python Setup
Added `actions/setup-python@v4` step before Qt installation in all workflows:
```yaml
- name: Set up Python
  uses: actions/setup-python@v4
  with:
    python-version: '3.11'
```

This ensures a consistent, working Python environment is available before the Qt installation action runs.

### 2. Fixed Qt Installation Parameters
Updated all Qt installation steps with proper parameters:

**Linux:**
```yaml
- name: Install Qt6
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.7.0'          # or '6.5.3' for build.yml
    host: 'linux'             # Added
    target: 'desktop'
    arch: 'linux_gcc_64'      # Changed from 'linux_x64'
    modules: 'qtbase qttools'
    cache: true
    setup-python: 'false'     # Added - use our Python setup
```

**Windows:**
```yaml
- name: Install Qt6
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.7.0'          # or '6.5.3' for build.yml
    host: 'windows'           # Added
    target: 'desktop'
    arch: 'win64_msvc2022_64'
    modules: 'qtbase qttools'
    cache: true               # Changed from false
    setup-python: 'false'     # Added - use our Python setup
```

**macOS:**
```yaml
- name: Install Qt6
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.5.3'
    host: 'mac'               # Added
    target: 'desktop'
    modules: 'qtbase qttools'
    cache: true               # Added
    setup-python: 'false'     # Added - use our Python setup
```

### 3. Removed Problematic Debug Steps
In `test-builds.yml`, removed the manual Python/aqtinstall installation steps that were:
- Installing aqtinstall manually (redundant with the action's internal installation)
- Causing failures when pip or aqtinstall had issues
- Creating complex fallback logic that was error-prone

### 4. Updated Debug Workflow
In `debug-qt-modules.yml`:
- Added Python setup step
- Added `pip upgrade` before installing aqtinstall
- Updated Qt version references to match main workflows

## Files Modified
1. `.github/workflows/build.yml` - Main multi-platform build workflow
2. `.github/workflows/build-linux.yml` - Linux-specific build workflow
3. `.github/workflows/build-windows.yml` - Windows-specific build workflow
4. `.github/workflows/test-builds.yml` - Test builds workflow
5. `.github/workflows/debug-qt-modules.yml` - Qt debugging workflow

## Key Improvements

### Consistency
- All workflows now use the same pattern for Python and Qt setup
- Standardized Qt version specifications and parameters
- Consistent use of `setup-python: 'false'` to avoid conflicts

### Reliability
- Explicit Python setup prevents environment-related failures
- Proper `host` and `arch` parameters ensure correct Qt packages are downloaded
- Enabled caching where appropriate to speed up builds and reduce download failures

### Maintainability
- Removed complex fallback logic in test workflows
- Simplified debug workflows
- Clear separation between Python setup and Qt installation

## Testing Recommendations
After these changes, test the workflows by:
1. Pushing to a branch to trigger `build-linux.yml` and `build-windows.yml`
2. Manually triggering `test-builds.yml` via workflow_dispatch
3. Creating a tag to test the full release workflow
4. For macOS, push to main/develop to trigger `build.yml`

## Additional Notes
- The `setup-python: 'false'` parameter tells the Qt installation action to use the Python environment we've already set up, rather than trying to set up its own
- Using Python 3.11 specifically as it's a stable, well-supported version on all GitHub Actions runners
- The `arch` parameter changes (e.g., `linux_x64` → `linux_gcc_64`) align with the actual Qt archive naming conventions used by aqtinstall
