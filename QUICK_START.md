# Quick Start Guide

## 🚀 Get Started in 5 Minutes

### Step 1: Build the Library

```bash
git clone <repository-url>
cd 3d-entity-manager
mkdir build && cd build
cmake ..
make
```

### Step 2: Include in Your Project

```cmake
# In your CMakeLists.txt
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(OpenSceneGraph REQUIRED)
find_package(osgEarth REQUIRED)

include_directories(path/to/3d-entity-manager/include)
target_link_libraries(your_target 3d-entity-manager ${QT_LIBRARIES} ${OSG_LIBRARIES})
```

### Step 3: Basic Usage

```cpp
#include "EntityManager.h"

// 1. Create entity manager
GlobalPulseTimeCallback* pulseCB = new GlobalPulseTimeCallback();
sceneRoot->addUpdateCallback(pulseCB);

EntityManager* manager = new EntityManager(
    sceneRoot, pulseCB, camera, nullptr);

// 2. Create entities
manager->createEntity(1, EntityState::SHIP, "./models/ship.osgb");
manager->createEntity(2, EntityState::MISSILE, "./models/missile.osgb");

// 3. Start rendering
manager->startRendering();

// 4. Update entities (from DDS or other source)
EntityState state;
state.entityId = 1;
state.lon = 140.0;
state.lat = 25.0;
state.alt = 0.0;
state.heading = 45.0;
manager->updateEntityState(state);
```

## 📋 Common Tasks

### Add Sensor Volume to Ship

```cpp
ShipModel* ship = new ShipModel(140, 25, 0, 1.0, "./models/ship.osgb");
SensorVolume* sensor = new SensorVolume(
    300000.0,                        // 300km range
    osg::Vec4(1.0, 0.0, 0.0, 0.3),  // Red, 30% opacity
    0.0, 120.0,                      // Azimuth range
    10.0, 90.0,                      // Elevation range
    20, 20);                         // Detail level
ship->addFixedWave(sensor);
```

### Add Track Line to Missile

```cpp
MissileModel* missile = new MissileModel(
    140, 30, 100000, 45, 90, 0, 50.0, "./models/missile.osgb");
    
TrackLine* track = new TrackLine(
    1000000.0,                       // 1000km length
    1000.0,                          // Radius
    osg::Vec4(1.0, 1.0, 0.0, 0.4),  // Yellow, 40% opacity
    100000.0, 5.0, 80);              // Width, speed, layers
    
pulseCB->addTrackLine(track);
missile->addRadarTrackLine(track);
```

### Enable Performance Monitoring

```cpp
manager->enablePerformanceStats(true);
// Console output: [EntityManager] FPS: 35.2 | Visible: 100 | Total: 200
```

### Toggle Component Visibility

```cpp
manager->setSensorVolumesVisible(false);  // Hide all sensors
manager->setTrackLinesVisible(false);     // Hide all track lines
```

### Batch Update (Efficient)

```cpp
QVector<EntityState> states;
for (const auto& ddsMsg : messages) {
    EntityState state;
    // ... fill from DDS message ...
    states.append(state);
}
manager->updateEntityStates(states);  // Single call for all
```

## ⚙️ Performance Tips

1. **Start with default LOD distances** - adjust only if needed
2. **Hide sensor volumes** if you have many entities (most expensive)
3. **Use batch updates** when updating multiple entities
4. **Enable performance stats** during development to monitor FPS
5. **Adjust detail levels** in LodConfig.h for your scene scale

## 🔧 Tuning Parameters

Edit `include/LodConfig.h`:

```cpp
// Distance thresholds (meters)
DISTANCE_NEAR = 500000.0;    // High detail below this
DISTANCE_MID  = 2000000.0;   // Medium detail
DISTANCE_FAR  = 5000000.0;   // Low detail (hidden beyond)

// Sensor detail (degrees per step)
SENSOR_AZI_STEP_HIGH = 10;   // More detail = more polygons
SENSOR_AZI_STEP_MID  = 20;
SENSOR_AZI_STEP_LOW  = 40;

// Track line detail (layers)
TRACKLINE_LAYERS_HIGH = 150;  // More layers = smoother
TRACKLINE_LAYERS_MID  = 80;
TRACKLINE_LAYERS_LOW  = 40;
```

## 🐛 Troubleshooting

### Models not appearing?
- Check model file paths are correct
- Verify models are in supported format (.osgb, .osg, .obj)
- Check console for loading errors

### Poor performance?
- Hide sensor volumes: `setSensorVolumesVisible(false)`
- Increase LOD distances in LodConfig.h
- Reduce detail levels (larger step angles, fewer layers)

### Track lines not animating?
- Ensure GlobalPulseTimeCallback is added to scene root
- Verify shader files exist in `./resource/osgEarth/`
- Check track lines are added to pulse callback

### Entities not updating?
- Verify `startRendering()` was called
- Check timestamp in EntityState is set
- Enable performance stats to see update counts

## 📚 Next Steps

- Read [README.md](README.md) for detailed documentation
- Check [examples/IntegrationExample.cpp](examples/IntegrationExample.cpp) for complete examples
- Review [IMPLEMENTATION_CHECKLIST.md](IMPLEMENTATION_CHECKLIST.md) for implementation details

## 🎯 Performance Goals

| Entities | Target FPS |
|----------|-----------|
| 10       | 40-50     |
| 50       | 30-35     |
| 100      | 20-25     |
| 200      | 15-20     |

With proper configuration and model optimization, these targets are achievable!
