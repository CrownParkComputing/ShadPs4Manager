#!/bin/bash

# Trigger GitHub Actions release workflow
# Usage: ./trigger-release.sh [patch|minor|major]

VERSION_BUMP="${1:-patch}"

if ! command -v gh &> /dev/null; then
    echo "GitHub CLI (gh) is not installed."
    echo ""
    echo "Please trigger the release manually:"
    echo "1. Go to: https://github.com/CrownParkComputing/ShadPs4Manager/actions/workflows/release.yml"
    echo "2. Click 'Run workflow'"
    echo "3. Select branch: main"
    echo "4. Choose version bump: $VERSION_BUMP"
    echo "5. Enable 'Create GitHub release'"
    echo "6. Click 'Run workflow'"
    echo ""
    echo "To install GitHub CLI: sudo apt install gh"
    exit 1
fi

echo "Triggering release workflow with $VERSION_BUMP version bump..."

gh workflow run release.yml \
    --ref main \
    -f version_bump="$VERSION_BUMP" \
    -f create_release=true

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Release workflow triggered successfully!"
    echo ""
    echo "Monitor progress at:"
    echo "https://github.com/CrownParkComputing/ShadPs4Manager/actions"
    echo ""
    echo "Current version: $(cat VERSION)"
    
    case "$VERSION_BUMP" in
        patch)
            echo "New version will be: $(cat VERSION | awk -F. '{printf "%d.%d.%d", $1, $2, $3+1}')"
            ;;
        minor)
            echo "New version will be: $(cat VERSION | awk -F. '{printf "%d.%d.0", $1, $2+1}')"
            ;;
        major)
            echo "New version will be: $(cat VERSION | awk -F. '{printf "%d.0.0", $1+1}')"
            ;;
    esac
else
    echo "❌ Failed to trigger release workflow"
    exit 1
fi
