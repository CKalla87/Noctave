# Adding the Nosferatu Image to Noctave

To display the Nosferatu image in the Noctave plugin UI, follow these steps:

## Option 1: Add as Resource File (Recommended)

1. **Place the image file:**
   - Copy your Nosferatu image to: `Noctave/Resources/nosferatu.png`
   - Supported formats: PNG, JPEG (PNG recommended for transparency)

2. **Open in Projucer:**
   - Open `Noctave.jucer` in Projucer
   - The image should appear in the Resources group
   - Ensure it's marked as a resource (not compiled code)

3. **Rebuild:**
   - Re-export the project to your IDE
   - Rebuild the plugin

## Option 2: Embed in BinaryData

1. **In Projucer:**
   - Go to the "Files" tab
   - Add your image to the BinaryData section
   - Projucer will generate `BinaryData.h` with the image data

2. **Update PluginEditor.cpp:**
   - The code already includes a fallback to load from BinaryData
   - If using BinaryData, the image will be automatically embedded

## Current Implementation

The plugin editor will attempt to load the image in this order:
1. From `Resources/nosferatu.png` (relative to executable)
2. From BinaryData (if embedded)

If the image is not found, a placeholder will be displayed with instructions.

## Image Recommendations

- **Size**: 250x400 pixels or similar aspect ratio works well
- **Format**: PNG with transparency for best results
- **Style**: The current UI is designed for dark, high-contrast images like the Nosferatu silhouette

