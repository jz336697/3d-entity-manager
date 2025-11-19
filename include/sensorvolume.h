#ifndef SENSORVOLUME_H
#define SENSORVOLUME_H

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/BlendFunc>
#include <osg/Depth>
#include "LodConfig.h"

/**
 * @file sensorvolume.h
 * @brief Sensor volume (radar coverage) with dynamic LOD support
 * 
 * Creates a 3D sector shape representing sensor coverage area.
 * Supports dynamic LOD to reduce polygon count based on camera distance.
 * 
 * LOD Levels:
 * - Level 0 (High): 10° steps (azimuth/elevation) - ~360 vertices
 * - Level 1 (Mid):  20° steps - ~90 vertices
 * - Level 2 (Low):  40° steps - ~24 vertices
 * 
 * Performance: LOD can reduce GPU load by 15x at distance.
 */

class SensorVolume : public osg::Referenced
{
public:
    /**
     * @brief Constructor
     * @param radius Sensor range in meters
     * @param color Color and transparency (RGBA)
     * @param azimuthStart Start azimuth angle in degrees (0 = North)
     * @param azimuthEnd End azimuth angle in degrees
     * @param elevationStart Start elevation angle in degrees (0 = horizon)
     * @param elevationEnd End elevation angle in degrees
     * @param azimuthStep Initial azimuth step size in degrees
     * @param elevationStep Initial elevation step size in degrees
     */
    SensorVolume(
        double radius,
        const osg::Vec4& color,
        double azimuthStart,
        double azimuthEnd,
        double elevationStart,
        double elevationEnd,
        int azimuthStep = LodConfig::SENSOR_AZI_STEP_MID,
        int elevationStep = LodConfig::SENSOR_ELE_STEP_MID
    );

    virtual ~SensorVolume();

    /**
     * @brief Get the geode containing the sensor volume geometry
     */
    osg::Geode* getGeode() { return m_geode.get(); }

    /**
     * @brief Set LOD level (0=high, 1=mid, 2=low)
     * Dynamically adjusts geometry detail based on distance
     */
    void setLodLevel(int level);
    int getLodLevel() const { return m_currentLodLevel; }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    /**
     * @brief Update sensor parameters
     */
    void setRadius(double radius);
    void setColor(const osg::Vec4& color);
    void setAngles(double azimuthStart, double azimuthEnd, 
                   double elevationStart, double elevationEnd);

protected:
    /**
     * @brief Rebuild geometry with current LOD level
     */
    void rebuildGeometry();

    /**
     * @brief Create vertices for the sensor volume
     */
    void createVertices(osg::Vec3Array* vertices, 
                       int azimuthStep, 
                       int elevationStep);

    // Sensor parameters
    double m_radius;
    osg::Vec4 m_color;
    double m_azimuthStart;
    double m_azimuthEnd;
    double m_elevationStart;
    double m_elevationEnd;

    // LOD
    int m_currentLodLevel;
    bool m_visible;

    // Geometry
    osg::ref_ptr<osg::Geode> m_geode;
    osg::ref_ptr<osg::Geometry> m_geometry;
};

#endif // SENSORVOLUME_H
