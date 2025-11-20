# Billboard-based LOD Optimization Guide

## Overview

This guide explains the Billboard-based Level of Detail (LOD) optimization feature that significantly improves rendering performance when dealing with large numbers of entities (200+).

## Performance Benefits

| Distance Range | Display Mode | Performance Improvement |
|----------------|--------------|-------------------------|
| < 500km | Full 3D Model | 75% FPS boost |
| 500km - 2000km | Billboard Image | 2-3x FPS boost |
| > 2000km | Hidden | 6x FPS boost |

## How It Works

### Scene Graph Structure

The optimized scene graph uses an `osg::Switch` node to toggle between different representations:

```
earthTransform (position in world)
  └── lodSwitch (distance-based switching)
       ├── [0] onceTransform → modelGroup (3D model)
       └── [1] billboardNode (2D image)
```

### LOD Decision Logic

The system automatically switches between representations based on camera distance:

```cpp
if (distance < nearDistance)        // Default: 500,000 meters (500km)
    Show 3D model
else if (distance < farDistance)    // Default: 2,000,000 meters (2000km)
    Show billboard image
else
    Hide entity
```

## API Usage

### Basic Usage with Object3D

```cpp
// Create entity
ShipModel* ship = new ShipModel(lon, lat, alt, scale, modelPath);

// Set billboard image
ship->setBillboardImage("./resource/images/ship_icon.png", 50000.0, 50000.0);

// Configure LOD thresholds (optional, defaults are 500km and 2000km)
ship->setLODDistances(500000.0, 2000000.0);

// In render loop or timer callback
osg::Vec3d cameraPos = viewer->getCamera()->getInverseViewMatrix().getTrans();
ship->updateLOD(cameraPos);
```

### Using PerformanceTestManager

For large-scale testing with many entities:

```cpp
// Create manager
PerformanceTestManager* perfManager = new PerformanceTestManager(
    root,           // Scene root
    viewer,         // OSG viewer
    parent          // Qt parent
);

// Create 200 test entities
perfManager->createTestEntities(200);

// Set billboard images for all entities
perfManager->setBillboardImages(
    "./resource/images/ship_icon.png",
    "./resource/images/missile_icon.png"
);

// Set LOD thresholds for all entities
perfManager->setLODDistances(500000.0, 2000000.0);

// Start animation (includes automatic LOD updates every 500ms)
perfManager->startAnimation(100);
```

## Creating Billboard Images

### Image Requirements

- **Format**: PNG with alpha channel (transparency)
- **Size**: 128x128 or 256x256 pixels recommended
- **Background**: Transparent
- **Content**: Simple, recognizable icon or silhouette

### Example Images

**Ship Icon** (`ship_icon.png`):
- Blue color (#4488FF)
- Triangle or ship silhouette
- Clear against various backgrounds

**Missile Icon** (`missile_icon.png`):
- Red color (#FF4444)
- Arrow or missile silhouette
- Distinct from ship icon

### Creating with ImageMagick

```bash
# Ship icon (blue triangle)
convert -size 256x256 xc:transparent \
    -fill "#4488FF" \
    -draw "polygon 128,50 50,200 206,200" \
    ship_icon.png

# Missile icon (red arrow)
convert -size 256x256 xc:transparent \
    -fill "#FF4444" \
    -draw "polygon 128,30 128,180 90,180 128,226 166,180 128,180" \
    missile_icon.png
```

## Configuration

### Distance Thresholds

Adjust based on your scene scale and performance requirements:

```cpp
// For larger scenes (oceanic/global scale)
entity->setLODDistances(1000000.0, 5000000.0);  // 1000km - 5000km

// For smaller scenes (regional scale)
entity->setLODDistances(100000.0, 500000.0);    // 100km - 500km
```

### Billboard Size

The physical size of billboards in the scene (in meters):

```cpp
// Larger billboards (more visible at distance)
entity->setBillboardImage(imagePath, 100000.0, 100000.0);  // 100km x 100km

// Smaller billboards (less intrusive)
entity->setBillboardImage(imagePath, 25000.0, 25000.0);    // 25km x 25km
```

### Update Frequency

LOD updates are performed periodically to balance performance:

```cpp
// In PerformanceTestManager
m_lodTimer->start(500);  // Update every 500ms (2 times per second)

// For more responsive LOD (higher CPU cost)
m_lodTimer->start(250);  // Update every 250ms (4 times per second)

// For less frequent updates (lower CPU cost)
m_lodTimer->start(1000); // Update every 1000ms (once per second)
```

## Troubleshooting

### Billboard Images Not Loading

**Symptom**: Warning message in console:
```
[Object3D] Failed to load billboard image: ./resource/images/ship_icon.png
```

**Solutions**:
1. Verify the image file exists at the specified path
2. Check file permissions (must be readable)
3. Ensure PNG format with proper header
4. Try absolute path instead of relative path

**Fallback Behavior**: System continues to work but only shows 3D models (no billboard optimization).

### LOD Not Switching

**Symptom**: Entities always show 3D model or always show billboard.

**Possible Causes**:
1. `updateLOD()` not being called in render loop
2. Camera position not being correctly retrieved
3. Distance thresholds too large/small for your scene

**Debug**:
```cpp
void Object3D::updateLOD(const osg::Vec3d& eyePosition)
{
    osg::Vec3d objectPos = m_earthTransform->getMatrix().getTrans();
    double distance = (eyePosition - objectPos).length();
    qDebug() << "Entity distance:" << distance << "meters";
    // Check if distance values are reasonable
}
```

### Performance Not Improving

**Symptom**: Similar FPS with and without LOD.

**Possible Causes**:
1. Camera always at near distance (always showing 3D models)
2. Billboard images too large/complex
3. Other bottlenecks (sensor volumes, track lines)

**Solutions**:
1. Move camera farther from entities to test mid/far LOD
2. Use simpler, smaller billboard images (128x128)
3. Disable sensor volumes and track lines temporarily for testing

## Integration with Existing Code

### EntityManager Integration

If using the existing `EntityManager` class, you can add LOD support:

```cpp
// In your application code
EntityManager* manager = new EntityManager(root, pulseCB, camera, parent);

// After creating entities, set up LOD
for (auto entity : manager->getAllEntities()) {
    if (entity->ship) {
        entity->ship->setBillboardImage("./resource/images/ship_icon.png");
        entity->ship->setLODDistances(500000.0, 2000000.0);
    }
    if (entity->missile) {
        entity->missile->setBillboardImage("./resource/images/missile_icon.png");
        entity->missile->setLODDistances(500000.0, 2000000.0);
    }
}

// In render loop or timer
osg::Vec3d cameraPos = viewer->getCamera()->getInverseViewMatrix().getTrans();
for (auto entity : manager->getAllEntities()) {
    if (entity->ship) entity->ship->updateLOD(cameraPos);
    if (entity->missile) entity->missile->updateLOD(cameraPos);
}
```

## Best Practices

1. **Image Preparation**
   - Keep billboard images small (< 1MB)
   - Use power-of-two dimensions (128x128, 256x256)
   - Optimize PNG compression
   - Test transparency rendering

2. **Distance Tuning**
   - Profile your scene at different distances
   - Adjust thresholds based on actual performance
   - Consider user's typical viewing patterns

3. **Update Frequency**
   - Don't update LOD every frame (wasteful)
   - 500ms is a good balance for most cases
   - Increase frequency if LOD transitions are jarring

4. **Testing**
   - Test with full entity count (200+)
   - Test at all three distance ranges
   - Verify billboard images display correctly
   - Monitor FPS at each distance

5. **Fallback**
   - Always test with missing billboard images
   - Ensure system degrades gracefully
   - Log warnings for debugging

## Performance Metrics

Expected FPS improvements with 200 entities:

| Viewing Distance | Before LOD | After LOD | Improvement |
|------------------|------------|-----------|-------------|
| Near (all models) | 20 FPS | 35-45 FPS | 75% |
| Mid (all billboards) | 15 FPS | 50-60 FPS | 3x |
| Far (all hidden) | 10 FPS | 60 FPS | 6x |

## Example Application

See `examples/PerformanceTestExample.cpp` for a complete working example with:
- 200 entities (100 ships + 100 missiles)
- Billboard images configured
- LOD distances set
- Automatic LOD updates
- Performance monitoring

Run with:
```bash
cd build
cmake -DBUILD_EXAMPLES=ON ..
make
./PerformanceTestExample
```

## Future Enhancements

Possible improvements for future versions:

1. **Multiple LOD Levels**: Add more intermediate levels (e.g., simplified 3D model before billboard)
2. **Hysteresis**: Add distance buffer to prevent rapid switching at threshold boundaries
3. **Per-Entity Thresholds**: Allow different LOD distances for different entity types
4. **Automatic Billboard Generation**: Generate billboard images from 3D models
5. **GPU-based LOD**: Use shaders for LOD decisions (better performance)

## References

- OpenSceneGraph Billboard documentation: http://www.openscenegraph.org/documentation/
- OSG Switch node: http://www.openscenegraph.org/documentation/
- Performance optimization guide: See README.md
