#ifndef TRACKLINE_H
#define TRACKLINE_H

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Program>
#include <osg/Uniform>
#include "LodConfig.h"

/**
 * @file trackline.h
 * @brief Track line (trajectory) with dynamic LOD and pulse animation
 * 
 * Creates an animated track line showing entity trajectory.
 * Supports dynamic LOD to reduce polygon count based on camera distance.
 * Uses shader-based pulse animation for visual effect.
 * 
 * LOD Levels:
 * - Level 0 (High): 150 layers - Very smooth
 * - Level 1 (Mid):  80 layers  - Moderate smooth
 * - Level 2 (Low):  40 layers  - Low smooth
 * 
 * Performance: Update frequency reduced (every 3rd position update)
 */

class TrackLine : public osg::Referenced
{
public:
    /**
     * @brief Constructor
     * @param length Track line length in meters
     * @param radius Track line radius/thickness in meters
     * @param color Color and transparency (RGBA)
     * @param width Visual width parameter
     * @param speed Animation speed parameter
     * @param layers Number of layers (smoothness)
     */
    TrackLine(
        double length,
        double radius,
        const osg::Vec4& color,
        double width = 100000.0,
        double speed = 5.0,
        int layers = LodConfig::TRACKLINE_LAYERS_MID
    );

    virtual ~TrackLine();

    /**
     * @brief Get the geode containing the track line geometry
     */
    osg::Geode* getGeode() { return m_geode.get(); }

    /**
     * @brief Set LOD level (0=high, 1=mid, 2=low)
     */
    void setLodLevel(int level);
    int getLodLevel() const { return m_currentLodLevel; }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }

    /**
     * @brief Update track line parameters
     */
    void setLength(double length);
    void setRadius(double radius);
    void setColor(const osg::Vec4& color);
    void setLayers(int layers);

    /**
     * @brief Update pulse animation time (called by GlobalPulseTimeCallback)
     */
    void setPulseTime(float time);

    /**
     * @brief Get shader uniforms for customization
     */
    osg::Uniform* getWidthUniform() { return m_widthUniform.get(); }
    osg::Uniform* getSpeedUniform() { return m_speedUniform.get(); }

protected:
    /**
     * @brief Rebuild geometry with current LOD level
     */
    void rebuildGeometry();

    /**
     * @brief Create vertices for the track line
     */
    void createVertices(osg::Vec3Array* vertices, int layers);

    /**
     * @brief Setup shader program for pulse animation
     */
    void setupShader();

    // Track line parameters
    double m_length;
    double m_radius;
    osg::Vec4 m_color;
    double m_width;
    double m_speed;
    int m_layers;

    // LOD
    int m_currentLodLevel;
    bool m_visible;

    // Geometry
    osg::ref_ptr<osg::Geode> m_geode;
    osg::ref_ptr<osg::Geometry> m_geometry;

    // Shader uniforms
    osg::ref_ptr<osg::Uniform> m_pulseTimeUniform;
    osg::ref_ptr<osg::Uniform> m_widthUniform;
    osg::ref_ptr<osg::Uniform> m_speedUniform;
    osg::ref_ptr<osg::Program> m_program;
};

#endif // TRACKLINE_H
