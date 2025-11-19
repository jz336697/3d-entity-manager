#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QDateTime>
#include <osg/Group>
#include <osg/Camera>
#include <osg/UpdateCallback>
#include "ShipModel.h"
#include "MissileModel.h"
#include "LodConfig.h"

/**
 * @file EntityManager.h
 * @brief Unified entity manager for high-performance rendering
 * 
 * Core component that manages all 3D entities (ships, missiles) with:
 * - Dynamic LOD based on camera distance
 * - Hierarchical update frequency (near entities update more frequently)
 * - Performance statistics tracking
 * - Batch updates for efficiency
 * 
 * Performance optimizations:
 * 1. Distance-based LOD (3 levels)
 * 2. Distance-based update frequency (3 levels)
 * 3. Frustum culling (entities outside view not updated)
 * 4. Dirty flag system (only update when data changes)
 */

// Entity state structure for DDS integration
struct EntityState {
    enum Type {
        SHIP,
        MISSILE
    };
    
    int entityId;
    Type type;
    
    // Position (WGS84)
    double lon;
    double lat;
    double alt;
    
    // Attitude (degrees)
    double heading;
    double pitch;
    double roll;
    
    // Timestamp
    qint64 timestamp;
    
    EntityState() 
        : entityId(-1)
        , type(SHIP)
        , lon(0), lat(0), alt(0)
        , heading(0), pitch(0), roll(0)
        , timestamp(0)
    {}
};

// Managed entity wrapper
struct ManagedEntity {
    int entityId;
    EntityState::Type type;
    osg::ref_ptr<Object3D> object;  // ShipModel or MissileModel
    
    // LOD management
    int lodLevel;           // Current LOD level (0=high, 1=mid, 2=low)
    double lastDistance;    // Distance to camera
    
    // Update management
    qint64 lastUpdateTime;  // Last update timestamp
    bool visible;           // Currently visible
    
    ManagedEntity()
        : entityId(-1)
        , type(EntityState::SHIP)
        , lodLevel(1)
        , lastDistance(0)
        , lastUpdateTime(0)
        , visible(true)
    {}
};

// Global pulse time callback for track line animation
class GlobalPulseTimeCallback : public osg::NodeCallback
{
public:
    GlobalPulseTimeCallback() : m_time(0.0f) {}
    
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        m_time += 0.016f; // Approximate 60 FPS increment
        
        // Update all registered track lines
        for (auto& trackLine : m_trackLines) {
            if (trackLine.valid()) {
                trackLine->setPulseTime(m_time);
            }
        }
        
        traverse(node, nv);
    }
    
    void addTrackLine(TrackLine* trackLine) {
        if (trackLine) {
            m_trackLines.push_back(trackLine);
        }
    }
    
    void clearTrackLines() {
        m_trackLines.clear();
    }
    
private:
    float m_time;
    QVector<osg::ref_ptr<TrackLine>> m_trackLines;
};

class EntityManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param sceneRoot Scene root node to add entities to
     * @param pulseCallback Global pulse callback for track lines
     * @param camera Camera for distance calculations
     * @param parent Qt parent object
     */
    EntityManager(
        osg::Group* sceneRoot,
        GlobalPulseTimeCallback* pulseCallback,
        osg::Camera* camera,
        QObject* parent = nullptr
    );

    virtual ~EntityManager();

    /**
     * @brief Create a new entity
     * @param entityId Unique entity identifier
     * @param type Entity type (SHIP or MISSILE)
     * @param modelPath Path to 3D model file
     * @return true if successful
     */
    bool createEntity(int entityId, EntityState::Type type, const QString& modelPath);

    /**
     * @brief Update entity state (position, attitude)
     * @param state New entity state
     */
    void updateEntityState(const EntityState& state);

    /**
     * @brief Batch update multiple entities (more efficient)
     * @param states Vector of entity states
     */
    void updateEntityStates(const QVector<EntityState>& states);

    /**
     * @brief Remove entity
     * @param entityId Entity identifier
     */
    void removeEntity(int entityId);

    /**
     * @brief Remove all entities
     */
    void clearAllEntities();

    /**
     * @brief Start automatic rendering updates
     * Updates LOD and performs hierarchical updates based on timer
     */
    void startRendering();

    /**
     * @brief Stop automatic rendering updates
     */
    void stopRendering();

    /**
     * @brief Enable/disable performance statistics output
     */
    void enablePerformanceStats(bool enable);

    /**
     * @brief Set visibility of sensor volumes
     */
    void setSensorVolumesVisible(bool visible);

    /**
     * @brief Set visibility of track lines
     */
    void setTrackLinesVisible(bool visible);

    /**
     * @brief Get entity count
     */
    int getEntityCount() const { return m_entities.size(); }

    /**
     * @brief Get visible entity count
     */
    int getVisibleEntityCount() const;

public slots:
    /**
     * @brief Update all entities (called by timer)
     */
    void updateAll();

protected:
    /**
     * @brief Update LOD for an entity based on camera distance
     * @param entity Entity to update
     * @return New LOD level
     */
    int updateEntityLod(ManagedEntity& entity);

    /**
     * @brief Calculate distance from camera to entity
     * @param entity Entity
     * @return Distance in meters
     */
    double calculateDistance(const ManagedEntity& entity);

    /**
     * @brief Check if entity should be updated this frame
     * @param entity Entity to check
     * @return true if should update
     */
    bool shouldUpdate(const ManagedEntity& entity) const;

    /**
     * @brief Print performance statistics
     */
    void printPerformanceStats();

private:
    osg::ref_ptr<osg::Group> m_sceneRoot;
    osg::ref_ptr<GlobalPulseTimeCallback> m_pulseCallback;
    osg::ref_ptr<osg::Camera> m_camera;
    
    QMap<int, ManagedEntity> m_entities;
    
    QTimer* m_updateTimer;
    bool m_performanceStatsEnabled;
    
    // Performance tracking
    qint64 m_lastStatsTime;
    int m_frameCount;
    
    // Visibility flags
    bool m_sensorVolumesVisible;
    bool m_trackLinesVisible;
};

#endif // ENTITYMANAGER_H
