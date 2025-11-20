#ifndef PERFORMANCETESTMANAGER_H
#define PERFORMANCETESTMANAGER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <osg/Group>
#include <osgViewer/Viewer>
#include "ShipModel.h"
#include "MissileModel.h"

/**
 * @file PerformanceTestManager.h
 * @brief Manager for performance testing with LOD optimization
 * 
 * This class manages a large number of entities for performance testing,
 * with support for distance-based LOD using Billboard images.
 */

class PerformanceTestManager : public QObject
{
    Q_OBJECT

public:
    struct EntityPair {
        osg::ref_ptr<ShipModel> ship;
        osg::ref_ptr<MissileModel> missile;
    };

    /**
     * @brief Constructor
     * @param root Scene root node
     * @param viewer OSG viewer for camera access
     * @param parent Qt parent object
     */
    PerformanceTestManager(
        osg::Group* root,
        osgViewer::Viewer* viewer,
        QObject* parent = nullptr
    );

    virtual ~PerformanceTestManager();

    /**
     * @brief Create test entities (ships and missiles)
     * @param count Number of entity pairs to create
     */
    void createTestEntities(int count);

    /**
     * @brief Set billboard images for all entities
     * @param shipImagePath Path to ship billboard image
     * @param missileImagePath Path to missile billboard image
     */
    void setBillboardImages(const QString& shipImagePath, const QString& missileImagePath);

    /**
     * @brief Set LOD distance thresholds for all entities
     * @param nearDist Distance threshold for near (show 3D model)
     * @param farDist Distance threshold for far (hide everything)
     */
    void setLODDistances(double nearDist, double farDist);

    /**
     * @brief Start animation and LOD updates
     * @param intervalMs Animation update interval in milliseconds
     */
    void startAnimation(int intervalMs = 100);

    /**
     * @brief Stop animation and LOD updates
     */
    void stopAnimation();
    
    /**
     * @brief Set global LOD mode
     * @param unifiedMode true=unified mode (switch based on camera altitude)
     *                    false=individual mode (each entity calculates distance independently)
     */
    void setGlobalLODMode(bool unifiedMode);
    
    /**
     * @brief Manually set global LOD level (only effective in unified mode)
     * @param level 0=all entities show 3D model, 1=all entities show image
     */
    void setGlobalLODLevel(int level);
    
    /**
     * @brief Get entity count
     */
    int getEntityCount() const { return m_entities.size(); }

private slots:
    /**
     * @brief Update entity positions (animation)
     */
    void updateAnimation();

    /**
     * @brief Update LOD based on camera distance
     */
    void updateLOD();

private:
    osg::Group* m_root;
    osgViewer::Viewer* m_viewer;
    QVector<EntityPair> m_entities;
    
    QTimer* m_animationTimer;
    QTimer* m_lodTimer;
    
    double m_animationTime;
    bool m_unifiedLODMode = true;  // Default: enable unified LOD mode
};

#endif // PERFORMANCETESTMANAGER_H
