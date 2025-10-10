# ShadPs4Manager Website

This directory contains the GitHub Pages website for ShadPs4Manager.

## Files

- `index.html` - Main website
- `assets/style.css` - Stylesheet
- `index.html.backup` - Original website backup

## GitHub Pages Setup

1. Go to repository **Settings** → **Pages**
2. Set source:
   - Branch: `main`
   - Folder: `/docs`
3. Save and wait 2-3 minutes

**Site URL**: https://crownparkcomputing.github.io/ShadPs4Manager/

## Local Preview

```bash
cd docs
python3 -m http.server 8000
# Visit: http://localhost:8000
```

## Updating

Edit `index.html` or `assets/style.css`, commit, and push.  
GitHub Pages auto-deploys in 1-2 minutes.
