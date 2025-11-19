#include "EntityManager.h"
#include <QDebug>
#include <cmath>

EntityManager::EntityManager(
    osg::Group* sceneRoot,
    GlobalPulseTimeCallback* pulseCallback,
    osg::Camera* camera,
    QObject* parent)
    : QObject(parent)
    , m_sceneRoot(sceneRoot)
    , m_pulseCallback(pulseCallback)
    , m_camera(camera)
    , m_performanceStatsEnabled(false)
    , m_lastStatsTime(0)
    , m_frameCount(0)
    , m_sensorVolumesVisible(true)
    , m_trackLinesVisible(true)
{
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &EntityManager::updateAll);
}

EntityManager::~EntityManager()
{
    clearAllEntities();
}

bool EntityManager::createEntity(int entityId, EntityState::Type type, const QString& modelPath)
{
    // Check if entity already exists
    if (m_entities.contains(entityId)) {
        qWarning() << "Entity" << entityId << "already exists";
        return false;
    }

    ManagedEntity managed;
    managed.entityId = entityId;
    managed.type = type;
    managed.lodLevel = 1; // Start with medium LOD
    managed.lastDistance = 0;
    managed.lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    managed.visible = true;

    // Create appropriate entity type
    if (type == EntityState::SHIP) {
        ShipModel* ship = new ShipModel(0, 0, 0, 1.0, modelPath);
        managed.object = ship;
        
        // Add to scene
        if (m_sceneRoot.valid()) {
            m_sceneRoot->addChild(ship->getModelTransform());
        }
    }
    else if (type == EntityState::MISSILE) {
        MissileModel* missile = new MissileModel(0, 0, 0, 0, 0, 0, 1.0, modelPath);
        managed.object = missile;
        
        // Add to scene
        if (m_sceneRoot.valid()) {
            m_sceneRoot->addChild(missile->getModelTransform());
        }
    }

    m_entities.insert(entityId, managed);
    return true;
}

void EntityManager::updateEntityState(const EntityState& state)
{
    if (!m_entities.contains(state.entityId)) {
        qWarning() << "Entity" << state.entityId << "not found";
        return;
    }

    ManagedEntity& entity = m_entities[state.entityId];
    
    // Update position and attitude
    if (entity.object.valid()) {
        entity.object->setPosition(state.lon, state.lat, state.alt);
        entity.object->setAttitude(state.heading, state.pitch, state.roll);
        entity.object->updateIfDirty();
    }
    
    entity.lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
}

void EntityManager::updateEntityStates(const QVector<EntityState>& states)
{
    // Batch update - more efficient than individual updates
    for (const EntityState& state : states) {
        updateEntityState(state);
    }
}

void EntityManager::removeEntity(int entityId)
{
    if (!m_entities.contains(entityId)) {
        return;
    }

    ManagedEntity& entity = m_entities[entityId];
    
    // Remove from scene
    if (entity.object.valid() && m_sceneRoot.valid()) {
        m_sceneRoot->removeChild(entity.object->getModelTransform());
    }
    
    m_entities.remove(entityId);
}

void EntityManager::clearAllEntities()
{
    QList<int> entityIds = m_entities.keys();
    for (int id : entityIds) {
        removeEntity(id);
    }
}

void EntityManager::startRendering()
{
    // Update at 20 Hz (50ms) - good balance between responsiveness and performance
    m_updateTimer->start(50);
    m_lastStatsTime = QDateTime::currentMSecsSinceEpoch();
    m_frameCount = 0;
}

void EntityManager::stopRendering()
{
    m_updateTimer->stop();
}

void EntityManager::enablePerformanceStats(bool enable)
{
    m_performanceStatsEnabled = enable;
    if (enable) {
        m_lastStatsTime = QDateTime::currentMSecsSinceEpoch();
        m_frameCount = 0;
    }
}

void EntityManager::setSensorVolumesVisible(bool visible)
{
    m_sensorVolumesVisible = visible;
    
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        ManagedEntity& entity = it.value();
        if (entity.type == EntityState::SHIP && entity.object.valid()) {
            ShipModel* ship = dynamic_cast<ShipModel*>(entity.object.get());
            if (ship) {
                ship->setSensorVolumesVisible(visible);
            }
        }
    }
}

void EntityManager::setTrackLinesVisible(bool visible)
{
    m_trackLinesVisible = visible;
    
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        ManagedEntity& entity = it.value();
        if (entity.type == EntityState::MISSILE && entity.object.valid()) {
            MissileModel* missile = dynamic_cast<MissileModel*>(entity.object.get());
            if (missile) {
                missile->setTrackLinesVisible(visible);
            }
        }
    }
}

int EntityManager::getVisibleEntityCount() const
{
    int count = 0;
    for (auto it = m_entities.constBegin(); it != m_entities.constEnd(); ++it) {
        if (it.value().visible) {
            ++count;
        }
    }
    return count;
}

void EntityManager::updateAll()
{
    if (!m_camera.valid()) {
        return;
    }

    int updatedCount = 0;
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Update all entities
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
        ManagedEntity& entity = it.value();
        
        if (!entity.object.valid()) {
            continue;
        }

        // Update LOD based on distance
        int newLodLevel = updateEntityLod(entity);
        
        // Check if entity is too far away (beyond FAR distance)
        if (entity.lastDistance > LodConfig::DISTANCE_FAR) {
            if (entity.visible) {
                entity.object->setVisible(false);
                entity.visible = false;
            }
            continue;
        }
        else {
            if (!entity.visible) {
                entity.object->setVisible(true);
                entity.visible = true;
            }
        }

        // Hierarchical update frequency based on LOD
        if (shouldUpdate(entity)) {
            // Update dirty transforms
            entity.object->updateIfDirty();
            
            // Update LOD for child components (sensors, track lines)
            if (entity.type == EntityState::SHIP) {
                ShipModel* ship = dynamic_cast<ShipModel*>(entity.object.get());
                if (ship) {
                    ship->updateSensorLod(newLodLevel);
                }
            }
            else if (entity.type == EntityState::MISSILE) {
                MissileModel* missile = dynamic_cast<MissileModel*>(entity.object.get());
                if (missile) {
                    missile->updateTrackLineLod(newLodLevel);
                }
            }
            
            entity.lastUpdateTime = now;
            updatedCount++;
        }
    }

    m_frameCount++;

    // Print performance statistics every second
    if (m_performanceStatsEnabled && (now - m_lastStatsTime) >= 1000) {
        printPerformanceStats();
        m_lastStatsTime = now;
        m_frameCount = 0;
    }
}

int EntityManager::updateEntityLod(ManagedEntity& entity)
{
    // Calculate distance to camera
    double distance = calculateDistance(entity);
    entity.lastDistance = distance;

    // Determine LOD level based on distance
    int newLodLevel;
    if (distance < LodConfig::DISTANCE_NEAR) {
        newLodLevel = 0; // High detail
    }
    else if (distance < LodConfig::DISTANCE_MID) {
        newLodLevel = 1; // Medium detail
    }
    else if (distance < LodConfig::DISTANCE_FAR) {
        newLodLevel = 2; // Low detail
    }
    else {
        newLodLevel = 3; // Very far - will be hidden
    }

    entity.lodLevel = newLodLevel;
    return newLodLevel;
}

double EntityManager::calculateDistance(const ManagedEntity& entity)
{
    if (!entity.object.valid() || !m_camera.valid()) {
        return 0.0;
    }

    // Get entity position in world coordinates
    osg::Vec3d entityPos = entity.object->getPosition();
    
    // Convert to ECEF
    osg::EllipsoidModel ellipsoid;
    osg::Vec3d ecef;
    ellipsoid.convertLatLongHeightToXYZ(
        osg::DegreesToRadians(entityPos.y()),
        osg::DegreesToRadians(entityPos.x()),
        entityPos.z(),
        ecef.x(), ecef.y(), ecef.z()
    );

    // Get camera position in world coordinates
    osg::Vec3d cameraPos = m_camera->getInverseViewMatrix().getTrans();

    // Calculate distance
    return (ecef - cameraPos).length();
}

bool EntityManager::shouldUpdate(const ManagedEntity& entity) const
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 interval;

    // Determine update interval based on LOD level
    switch (entity.lodLevel) {
        case 0: // Near - high frequency
            interval = LodConfig::UPDATE_INTERVAL_NEAR;
            break;
        case 1: // Mid - medium frequency
            interval = LodConfig::UPDATE_INTERVAL_MID;
            break;
        case 2: // Far - low frequency
            interval = LodConfig::UPDATE_INTERVAL_FAR;
            break;
        default:
            return false; // Don't update if too far
    }

    return (now - entity.lastUpdateTime) >= interval;
}

void EntityManager::printPerformanceStats()
{
    double fps = m_frameCount / 1.0; // Approximate FPS (measured per second)
    int visibleCount = getVisibleEntityCount();
    int totalCount = m_entities.size();

    qDebug() << QString("[EntityManager] FPS: %1 | Visible: %2 | Total: %3")
        .arg(fps, 0, 'f', 1)
        .arg(visibleCount)
        .arg(totalCount);
}
