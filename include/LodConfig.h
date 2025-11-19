#ifndef LODCONFIG_H
#define LODCONFIG_H

/**
 * @file LodConfig.h
 * @brief LOD (Level of Detail) configuration parameters for performance optimization
 * 
 * This file defines distance thresholds and detail levels for dynamic LOD system.
 * Adjust these values based on your scene scale and performance requirements.
 */

namespace LodConfig {

// Distance thresholds (in meters)
static constexpr double DISTANCE_NEAR = 500000.0;    // 500km - High detail
static constexpr double DISTANCE_MID  = 2000000.0;   // 2000km - Medium detail
static constexpr double DISTANCE_FAR  = 5000000.0;   // 5000km - Low detail

// Sensor Volume LOD parameters
// Azimuth step angles (degrees)
static constexpr int SENSOR_AZI_STEP_HIGH = 10;   // LOD 0: High detail
static constexpr int SENSOR_AZI_STEP_MID  = 20;   // LOD 1: Medium detail
static constexpr int SENSOR_AZI_STEP_LOW  = 40;   // LOD 2: Low detail

// Elevation step angles (degrees)
static constexpr int SENSOR_ELE_STEP_HIGH = 10;   // LOD 0: High detail
static constexpr int SENSOR_ELE_STEP_MID  = 20;   // LOD 1: Medium detail
static constexpr int SENSOR_ELE_STEP_LOW  = 40;   // LOD 2: Low detail

// Track Line LOD parameters
// Number of layers (smoothness)
static constexpr int TRACKLINE_LAYERS_HIGH = 150;  // LOD 0: Very smooth
static constexpr int TRACKLINE_LAYERS_MID  = 80;   // LOD 1: Moderate smooth
static constexpr int TRACKLINE_LAYERS_LOW  = 40;   // LOD 2: Low smooth

// Update frequency (milliseconds)
static constexpr qint64 UPDATE_INTERVAL_NEAR = 50;   // 20 updates/sec - Near entities
static constexpr qint64 UPDATE_INTERVAL_MID  = 100;  // 10 updates/sec - Mid entities
static constexpr qint64 UPDATE_INTERVAL_FAR  = 200;  // 5 updates/sec - Far entities

// Performance tuning
static constexpr double POSITION_EPSILON = 1e-9;     // Minimum position change threshold
static constexpr double ATTITUDE_EPSILON = 1e-6;     // Minimum attitude change threshold
static constexpr int TRACKLINE_UPDATE_SKIP = 3;      // Update track line every N position updates

} // namespace LodConfig

#endif // LODCONFIG_H
