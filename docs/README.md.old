# ShadPs4Manager Website

This directory contains the GitHub Pages website for ShadPs4Manager.

## Website Structure

```
docs/
├── index.html          # Main website
├── assets/
│   └── style.css       # Stylesheet
└── README.md           # This file
```

## GitHub Pages Setup

### Enabling GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** → **Pages**
3. Under "Source", select:
   - **Branch**: `main`
   - **Folder**: `/docs`
4. Click **Save**
5. Wait a few minutes for deployment

Your site will be available at:
```
https://crownparkcomputing.github.io/ShadPs4Manager/
```

### Custom Domain (Optional)

If you have a custom domain:

1. In Pages settings, enter your custom domain
2. Create a `CNAME` file in this directory containing your domain
3. Add DNS records (A or CNAME) pointing to GitHub Pages

## Local Development

To preview the website locally:

### Using Python
```bash
cd docs
python3 -m http.server 8000
```

Then visit: http://localhost:8000

### Using PHP
```bash
cd docs
php -S localhost:8000
```

### Using Node.js (http-server)
```bash
npm install -g http-server
cd docs
http-server
```

## Updating the Website

### Adding New Content

1. Edit `index.html` to add new sections
2. Update `assets/style.css` for styling changes
3. Commit and push to GitHub
4. GitHub Pages will automatically rebuild (takes 1-2 minutes)

### Updating Download Links

When releasing a new version:

1. Update version number in `index.html`:
   ```html
   <h3>Latest Release: v1.0.2</h3>
   ```

2. Update release notes link if needed

3. Commit and push changes

### Adding Images

1. Create `docs/assets/images/` directory
2. Add images (screenshots, logos, etc.)
3. Reference in HTML:
   ```html
   <img src="assets/images/screenshot.png" alt="Description">
   ```

## Features Highlighted

The website showcases:

- ✅ Three applications (GUI, CLI, Admin Tool)
- ✅ Key features and capabilities
- ✅ Download links for all platforms
- ✅ Documentation links
- ✅ Trial information (10-game limit)
- ✅ Open source licensing (GPL-3.0)
- ✅ Build from source instructions

## Responsive Design

The website is fully responsive and works on:
- Desktop (1920x1080 and higher)
- Laptop (1366x768)
- Tablet (768x1024)
- Mobile (375x667)

## Browser Compatibility

Tested on:
- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

## Performance

The website is optimized for:
- Fast loading (< 1 second on 3G)
- No external dependencies
- Minimal CSS (self-contained)
- No JavaScript required

## SEO Considerations

Key SEO elements included:
- Semantic HTML5 structure
- Meta descriptions
- Proper heading hierarchy
- Alt text for images (when added)

To improve SEO, consider adding:
```html
<meta name="description" content="Professional PS4 Package Management Suite">
<meta name="keywords" content="PS4, PKG, extractor, manager, ShadPs4">
<meta property="og:title" content="ShadPs4Manager">
<meta property="og:description" content="Professional PS4 Package Management Suite">
<meta property="og:image" content="assets/images/og-image.png">
```

## Analytics (Optional)

To add Google Analytics:

1. Get your GA4 tracking ID
2. Add to `index.html` before `</head>`:
   ```html
   <script async src="https://www.googletagmanager.com/gtag/js?id=G-XXXXXXXXXX"></script>
   <script>
     window.dataLayer = window.dataLayer || [];
     function gtag(){dataLayer.push(arguments);}
     gtag('js', new Date());
     gtag('config', 'G-XXXXXXXXXX');
   </script>
   ```

## Troubleshooting

### Site Not Updating

If changes don't appear:
1. Clear browser cache (Ctrl+F5)
2. Check GitHub Actions for build errors
3. Wait 5-10 minutes for propagation
4. Verify correct branch/folder in Settings

### 404 Errors

If you get 404 errors:
1. Ensure `index.html` is in the root of `docs/`
2. Check Pages settings are correct
3. Verify repository is public

### CSS Not Loading

If styling doesn't appear:
1. Check `assets/style.css` exists
2. Verify path in `index.html`: `<link rel="stylesheet" href="assets/style.css">`
3. Check browser console for errors

## Maintenance

### Regular Updates

- Update download links with each release
- Keep documentation links current
- Add new features to feature list
- Update screenshots periodically

### Monitoring

Check regularly:
- Broken links
- GitHub Pages build status
- User feedback in issues

## Contributing

To contribute to the website:

1. Fork the repository
2. Make changes in the `docs/` directory
3. Test locally
4. Submit a pull request

Please maintain:
- Consistent styling
- Responsive design
- Fast load times
- Accessibility standards

## License

The website content is part of ShadPs4Manager and licensed under GPL-3.0.
