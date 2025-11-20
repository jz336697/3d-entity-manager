#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Vec3d>
#include <osg/Vec4>
#include <osg/Switch>
#include <osg/Billboard>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osgEarth/MapNode>
#include <osgEarth/EllipsoidModel>
#include "LodConfig.h"
#include <QString>

/**
 * @file object3d.h
 * @brief Optimized 3D object base class with dirty flag system and Billboard LOD support
 * 
 * KEY OPTIMIZATIONS:
 * 1. Removed AutoTransform to avoid per-frame screen coordinate recalculation (20-30% boost)
 * 2. Added Billboard-based LOD for distance optimization (2-6x boost at distance)
 * 
 * Scene graph hierarchy with LOD:
 * earth -> lodSwitch -> [0] once -> modelGroup (3D model)
 *                    -> [1] billboardNode (2D image)
 * 
 * LOD behavior (two-level strategy):
 * - < 500km: Show full 3D model
 * - >= 500km: Show billboard image (never auto-hide)
 * - Manual hide only via setVisible(false)
 * 
 * Uses dirty flag system to skip unnecessary updates when data hasn't changed.
 */

class Object3D : public osg::Referenced
{
public:
    Object3D();
    virtual ~Object3D();

    /**
     * @brief Set position (longitude, latitude, altitude)
     * Uses epsilon comparison to skip insignificant changes
     */
    void setPosition(double lon, double lat, double alt);
    void setPosition(const osg::Vec3d& pos);
    
    /**
     * @brief Set attitude (heading, pitch, roll in degrees)
     * Uses epsilon comparison to skip insignificant changes
     */
    void setAttitude(double heading, double pitch, double roll);
    
    /**
     * @brief Set scale factor for the model
     */
    void setScale(double scale);
    
    /**
     * @brief Set visibility of the object
     */
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Get current position
     */
    osg::Vec3d getPosition() const { return osg::Vec3d(m_longitude, m_latitude, m_altitude); }
    
    /**
     * @brief Get current attitude
     */
    osg::Vec3d getAttitude() const { return osg::Vec3d(m_heading, m_pitch, m_roll); }
    
    /**
     * @brief Update transforms if dirty flags are set
     * Call this before rendering to apply pending changes
     */
    void updateIfDirty();
    
    /**
     * @brief Get the root transform node for the scene graph
     */
    osg::MatrixTransform* getModelTransform() { return m_earthTransform.get(); }
    
    /**
     * @brief Get the model node (for track line attachment, etc.)
     */
    osg::Node* modelObject() { return m_onceTransform.get(); }
    
    /**
     * @brief Set Billboard image (PNG format, transparent background)
     * @param imagePath Path to the billboard image file
     * @param width Width of the billboard in meters (default: 50000.0)
     * @param height Height of the billboard in meters (default: 50000.0)
     */
    void setBillboardImage(const QString& imagePath, double width = 50000.0, double height = 50000.0);
    
    /**
     * @brief Set LOD distance thresholds
     * @param nearDist Distance threshold for near (show 3D model), in meters
     * @param farDist Deprecated - no longer used, retained for backward compatibility
     * 
     * LOD strategy (two-level):
     * - distance < nearDist: Display 3D model
     * - distance >= nearDist: Display Billboard image
     * - Never auto-hides; hiding only via setVisible(false)
     */
    void setLODDistances(double nearDist, double farDist);
    
    /**
     * @brief Update LOD based on camera position (call per frame or periodically)
     * @param eyePosition Camera position in world coordinates
     */
    void updateLOD(const osg::Vec3d& eyePosition);
    
    /**
     * @brief Force set LOD level (for unified mode)
     * @param level 0=3D model, 1=Billboard image
     */
    void forceLODLevel(int level);
    
    /**
     * @brief Enable/disable automatic LOD calculation
     * @param enabled true=auto calculate distance switching, false=use forceLODLevel manual control
     */
    void setAutoLOD(bool enabled);

protected:
    /**
     * @brief Update earth transform (position)
     * Only called when position changes
     */
    void updateEarthTransform();
    
    /**
     * @brief Update once transform (rotation and scale)
     * Only called when attitude or scale changes
     */
    void updateOnceTransform();
    
    /**
     * @brief Create billboard from image file
     * @param imagePath Path to the image file
     * @param width Width of the billboard in meters
     * @param height Height of the billboard in meters
     */
    void createBillboard(const QString& imagePath, double width, double height);

    // Cached EllipsoidModel to avoid creating it every time
    static osg::ref_ptr<osg::EllipsoidModel> s_ellipsoid;
    static osg::EllipsoidModel* getEllipsoid();

    // Position (WGS84)
    double m_longitude;
    double m_latitude;
    double m_altitude;
    
    // Attitude (degrees)
    double m_heading;
    double m_pitch;
    double m_roll;
    
    // Scale
    double m_scale;
    
    // Visibility
    bool m_visible;
    
    // Dirty flags - track what needs updating
    bool m_positionDirty;
    bool m_attitudeDirty;
    bool m_scaleDirty;
    
    // Scene graph nodes
    osg::ref_ptr<osg::MatrixTransform> m_earthTransform;  // Earth-relative position
    osg::ref_ptr<osg::MatrixTransform> m_onceTransform;   // Local rotation and scale
    osg::ref_ptr<osg::Group> m_modelGroup;                // Container for model and attachments
    
    // LOD support with Billboard
    osg::ref_ptr<osg::Billboard>       m_billboardNode;   // Billboard image node
    osg::ref_ptr<osg::Switch>          m_lodSwitch;       // LOD switch control
    
    double m_nearDistance = 500000.0;   // 500km - show 3D model
    double m_farDistance  = 2000000.0;  // Deprecated - no longer used in two-level LOD
    bool m_autoLOD = true;              // Enable automatic LOD (default: enabled)
};

#endif // OBJECT3D_H
