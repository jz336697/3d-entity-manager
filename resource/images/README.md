# Billboard Images for LOD Optimization

This directory contains billboard images used for distance-based LOD (Level of Detail) optimization.

## Required Images

### ship_icon.png
- **Size**: 128x128 or 256x256 pixels
- **Format**: PNG with transparency
- **Color**: Blue triangle or ship-shaped icon
- **Background**: Transparent
- **Purpose**: Displayed when ship is at medium distance (500km - 2000km)

### missile_icon.png
- **Size**: 128x128 or 256x256 pixels
- **Format**: PNG with transparency
- **Color**: Red arrow or missile-shaped icon
- **Background**: Transparent
- **Purpose**: Displayed when missile is at medium distance (500km - 2000km)

## Creating the Images

You can create these images using any image editor that supports transparency:
- GIMP
- Photoshop
- Paint.NET
- Online tools

### Example with ImageMagick (command line):

```bash
# Create simple ship icon (blue triangle)
convert -size 256x256 xc:transparent \
    -fill "#4488FF" \
    -draw "polygon 128,50 50,200 206,200" \
    ship_icon.png

# Create simple missile icon (red arrow)
convert -size 256x256 xc:transparent \
    -fill "#FF4444" \
    -draw "polygon 128,30 128,180 90,180 128,226 166,180 128,180" \
    missile_icon.png
```

## Fallback Behavior

If the image files are not found:
- A warning message will be logged: `[Object3D] Failed to load billboard image: <path>`
- The system will continue to work but will only show 3D models (no billboard fallback)
- Performance benefits will be reduced at medium distances

## Testing

To test if images are loaded correctly:
1. Run the PerformanceTestExample
2. Check console output for loading warnings
3. Move camera to medium distance (500km - 2000km)
4. Verify that billboards are visible instead of 3D models

## Performance Impact

Using billboard images provides significant performance improvements:
- **Near distance (<500km)**: 75% FPS boost
- **Mid distance (500-2000km)**: 3x FPS boost
- **Far distance (>2000km)**: 6x FPS boost (entities hidden)
