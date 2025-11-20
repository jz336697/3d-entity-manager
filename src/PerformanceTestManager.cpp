#include "PerformanceTestManager.h"
#include <QDebug>
#include <cmath>

PerformanceTestManager::PerformanceTestManager(
    osg::Group* root,
    osgViewer::Viewer* viewer,
    QObject* parent)
    : QObject(parent)
    , m_root(root)
    , m_viewer(viewer)
    , m_animationTime(0.0)
{
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &PerformanceTestManager::updateAnimation);
    
    m_lodTimer = new QTimer(this);
    connect(m_lodTimer, &QTimer::timeout, this, &PerformanceTestManager::updateLOD);
    
    qDebug() << "[PerformanceTestManager] Initialized with LOD support";
}

PerformanceTestManager::~PerformanceTestManager()
{
    stopAnimation();
}

void PerformanceTestManager::createTestEntities(int count)
{
    qDebug() << "[PerformanceTestManager] Creating" << count << "test entities...";
    
    for (int i = 0; i < count; ++i)
    {
        EntityPair entity;
        
        // Calculate positions in a grid pattern
        int gridSize = static_cast<int>(std::sqrt(count)) + 1;
        int row = i / gridSize;
        int col = i % gridSize;
        
        double lonBase = 140.0 + (col * 0.5);
        double latBase = 25.0 + (row * 0.5);
        
        // Create ship
        entity.ship = new ShipModel(
            lonBase,
            latBase,
            -90.0,  // Below sea level
            1.0,    // Scale
            "./models/ship.osgb"
        );
        m_root->addChild(entity.ship->getModelTransform());
        
        // Create missile
        entity.missile = new MissileModel(
            lonBase,
            latBase + 0.1,
            100000.0,  // 100km altitude
            45.0,      // Heading
            90.0,      // Pitch
            0.0,       // Roll
            50.0,      // Scale
            "./models/missile.osgb"
        );
        m_root->addChild(entity.missile->getModelTransform());
        
        m_entities.append(entity);
    }
    
    qDebug() << "[PerformanceTestManager] Successfully created" << count << "entities";
    
    // Apply unified LOD mode by default
    setGlobalLODMode(m_unifiedLODMode);
}

void PerformanceTestManager::setBillboardImages(const QString& shipImagePath, const QString& missileImagePath)
{
    qDebug() << "[PerformanceTestManager] Setting billboard images...";
    qDebug() << "  Ship image:" << shipImagePath;
    qDebug() << "  Missile image:" << missileImagePath;
    
    for (auto& entity : m_entities)
    {
        if (entity.ship && entity.ship->modelObject())
        {
            entity.ship->setBillboardImage(shipImagePath, 50000.0, 50000.0);
        }
        
        if (entity.missile && entity.missile->modelObject())
        {
            entity.missile->setBillboardImage(missileImagePath, 30000.0, 30000.0);
        }
    }
    
    qDebug() << "[PerformanceTestManager] Billboard images set for" << m_entities.size() << "entities";
}

void PerformanceTestManager::setLODDistances(double nearDist, double farDist)
{
    qDebug() << "[PerformanceTestManager] Setting LOD distances:";
    qDebug() << "  Near (show model):" << nearDist << "meters";
    qDebug() << "  Far (hide all):" << farDist << "meters";
    
    for (auto& entity : m_entities)
    {
        if (entity.ship && entity.ship->modelObject())
            entity.ship->setLODDistances(nearDist, farDist);
        
        if (entity.missile && entity.missile->modelObject())
            entity.missile->setLODDistances(nearDist, farDist);
    }
}

void PerformanceTestManager::startAnimation(int intervalMs)
{
    qDebug() << "[PerformanceTestManager] Starting animation (interval:" << intervalMs << "ms)";
    
    m_animationTimer->start(intervalMs);
    m_lodTimer->start(500);  // Update LOD every 500ms
    
    qDebug() << "[PerformanceTestManager] Animation started with LOD update";
}

void PerformanceTestManager::stopAnimation()
{
    if (m_animationTimer->isActive()) {
        m_animationTimer->stop();
        m_lodTimer->stop();
        qDebug() << "[PerformanceTestManager] Animation stopped";
    }
}

void PerformanceTestManager::updateAnimation()
{
    m_animationTime += 0.1;
    
    // Simple circular motion animation
    for (int i = 0; i < m_entities.size(); ++i)
    {
        auto& entity = m_entities[i];
        
        if (entity.ship)
        {
            osg::Vec3d pos = entity.ship->getPosition();
            double offset = std::sin(m_animationTime + i * 0.1) * 0.01;
            entity.ship->setPosition(pos.x() + offset, pos.y(), pos.z());
            entity.ship->updateIfDirty();
        }
        
        if (entity.missile)
        {
            osg::Vec3d pos = entity.missile->getPosition();
            double offset = std::cos(m_animationTime + i * 0.1) * 0.01;
            entity.missile->setPosition(pos.x(), pos.y() + offset, pos.z());
            entity.missile->updateIfDirty();
        }
    }
}

void PerformanceTestManager::updateLOD()
{
    if (!m_viewer || !m_viewer->getCamera())
        return;

    osg::Vec3d eyePos = m_viewer->getCamera()->getInverseViewMatrix().getTrans();

    if (!m_unifiedLODMode)
    {
        // Individual mode: each entity calculates distance independently (original way)
        for (auto& entity : m_entities)
        {
            if (entity.ship && entity.ship->modelObject())
                entity.ship->updateLOD(eyePos);
            
            if (entity.missile && entity.missile->modelObject())
                entity.missile->updateLOD(eyePos);
        }
    }
    else
    {
        // Unified mode: switch all entities uniformly based on camera altitude
        
        // Calculate camera altitude (distance from earth surface)
        // Earth radius approximately 6371000 meters
        const double EARTH_RADIUS = 6371000.0;
        double cameraAltitude = eyePos.length() - EARTH_RADIUS;
        
        // Determine global LOD level based on camera altitude
        // altitude < 500km: show 3D model
        // altitude >= 500km: show Billboard image
        int globalLevel = (cameraAltitude < 500000.0) ? 0 : 1;
        
        // Set unified LOD level for all entities
        setGlobalLODLevel(globalLevel);
    }
}

void PerformanceTestManager::setGlobalLODMode(bool unifiedMode)
{
    m_unifiedLODMode = unifiedMode;
    
    qDebug() << "[PerformanceTestManager] Setting global LOD mode:" 
             << (unifiedMode ? "Unified" : "Individual");
    
    // Disable/enable auto LOD for each entity
    for (auto& entity : m_entities)
    {
        if (entity.ship && entity.ship->modelObject())
            entity.ship->setAutoLOD(!unifiedMode);
        
        if (entity.missile && entity.missile->modelObject())
            entity.missile->setAutoLOD(!unifiedMode);
    }
    
    qDebug() << "[PerformanceTestManager] Auto LOD" 
             << (unifiedMode ? "disabled" : "enabled") 
             << "for all entities";
}

void PerformanceTestManager::setGlobalLODLevel(int level)
{
    for (auto& entity : m_entities)
    {
        if (entity.ship && entity.ship->modelObject())
            entity.ship->forceLODLevel(level);
        
        if (entity.missile && entity.missile->modelObject())
            entity.missile->forceLODLevel(level);
    }
}
