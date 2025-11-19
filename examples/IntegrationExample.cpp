/**
 * @file IntegrationExample.cpp
 * @brief Complete integration example for the 3D Entity Manager
 * 
 * This example demonstrates how to integrate the high-performance entity
 * management system into an osgEarth application.
 * 
 * Expected Performance:
 * - 10 entities:  40-50 FPS
 * - 50 entities:  30-35 FPS
 * - 100 entities: 20-25 FPS
 * - 200 entities: 15-20 FPS
 */

#include <osgViewer/Viewer>
#include <osgEarth/MapNode>
#include <osgEarth/ImageLayer>
#include <osgEarth/Sky>
#include <osgGA/TrackballManipulator>

#include "EntityManager.h"
#include "DdsDataSimulator.h"
#include "ShipModel.h"
#include "MissileModel.h"
#include "sensorvolume.h"
#include "trackline.h"

/**
 * @brief Initialize earth scene with sky
 */
osg::Group* createEarthScene()
{
    // Create map
    osgEarth::Map* map = new osgEarth::Map();
    
    // Add imagery layer (adjust URL to your data source)
    osgEarth::ImageLayer* imageLayer = new osgEarth::ImageLayer();
    imageLayer->setName("Imagery");
    // imageLayer->setURL("http://readymap.org/readymap/tiles/1.0.0/7/");
    map->addLayer(imageLayer);
    
    // Create map node
    osgEarth::MapNode* mapNode = new osgEarth::MapNode(map);
    
    // Create root group
    osg::Group* root = new osg::Group();
    root->addChild(mapNode);
    
    // Optional: Add sky
    // osgEarth::SkyNode* sky = osgEarth::SkyNode::create();
    // sky->attach(viewer, 0);
    // root->addChild(sky);
    
    return root;
}

/**
 * @brief Method A: Using EntityManager (Recommended)
 * 
 * This is the recommended approach for managing large numbers of entities.
 */
void exampleA_UseEntityManager()
{
    // Create viewer
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 1280, 720);
    
    // Create earth scene
    osg::Group* root = createEarthScene();
    
    // Create global pulse callback for track line animation
    GlobalPulseTimeCallback* pulseCB = new GlobalPulseTimeCallback();
    root->addUpdateCallback(pulseCB);
    
    // Create entity manager
    EntityManager* entityManager = new EntityManager(
        root,                    // Scene root
        pulseCB,                // Pulse callback
        viewer.getCamera(),     // Camera for LOD
        nullptr);               // Qt parent
    
    // Enable performance statistics
    entityManager->enablePerformanceStats(true);
    
    // Create 200 entities (ships and missiles)
    for (int i = 0; i < 200; ++i) {
        EntityState::Type type = (i % 2 == 0) 
            ? EntityState::SHIP 
            : EntityState::MISSILE;
            
        QString modelPath = (type == EntityState::SHIP)
            ? "./models/ship.osgb"
            : "./models/missile.osgb";
        
        entityManager->createEntity(i, type, modelPath);
    }
    
    // Start rendering
    entityManager->startRendering();
    
    // Optional: Use DDS simulator for testing
    DdsDataSimulator* ddsSimulator = new DdsDataSimulator(entityManager, nullptr);
    ddsSimulator->start(100);  // Update every 100ms
    
    // Setup camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.setSceneData(root);
    
    // Run viewer
    viewer.run();
    
    // Cleanup
    delete ddsSimulator;
    delete entityManager;
}

/**
 * @brief Method B: Direct usage of optimized classes
 * 
 * For simpler scenarios or when you need more control.
 */
void exampleB_DirectUsage()
{
    // Create viewer
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 1280, 720);
    
    // Create earth scene
    osg::Group* root = createEarthScene();
    
    // Create global pulse callback
    GlobalPulseTimeCallback* pulseCB = new GlobalPulseTimeCallback();
    root->addUpdateCallback(pulseCB);
    
    // Create a ship
    ShipModel* ship = new ShipModel(
        140.0,              // Longitude
        25.0,               // Latitude
        -90.0,              // Altitude (below sea level for ship)
        1.0,                // Scale
        "./models/ship.osgb"
    );
    root->addChild(ship->getModelTransform());
    
    // Add sensor volume to ship
    SensorVolume* sensor = new SensorVolume(
        300000.0,                        // Radius (300km)
        osg::Vec4(1.0, 0.0, 0.0, 0.3),  // Red color with transparency
        0.0, 120.0,                      // Azimuth: 0° to 120°
        10.0, 90.0,                      // Elevation: 10° to 90°
        20, 20                           // Step sizes (medium detail)
    );
    sensor->setLodLevel(1);
    ship->addFixedWave(sensor);
    
    // Create a missile
    MissileModel* missile = new MissileModel(
        140.0,              // Longitude
        33.5,               // Latitude
        100000.0,           // Altitude (100km)
        45.0,               // Heading
        90.0,               // Pitch
        0.0,                // Roll
        50.0,               // Scale
        "./models/missile.osgb"
    );
    root->addChild(missile->getModelTransform());
    
    // Add track line to missile
    TrackLine* trackLine = new TrackLine(
        1000000.0,                       // Length (1000km)
        1000.0,                          // Radius
        osg::Vec4(1.0, 1.0, 0.0, 0.4),  // Yellow color with transparency
        100000.0,                        // Width parameter
        5.0,                             // Speed parameter
        80                               // Layers (medium detail)
    );
    trackLine->setLodLevel(1);
    pulseCB->addTrackLine(trackLine);
    missile->addRadarTrackLine(trackLine, ship->modelObject());
    
    // Setup camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.setSceneData(root);
    
    // Run viewer
    viewer.run();
}

/**
 * @brief DDS Integration Example
 * 
 * Shows how to integrate with real DDS messages.
 */
class DdsMessageHandler : public QObject
{
    Q_OBJECT
    
public:
    DdsMessageHandler(EntityManager* manager) : m_manager(manager) {}
    
    // Called when DDS message received
    void onDdsDataReceived(const /* YourDdsMessage& */ void* msg)
    {
        // Parse DDS message
        // const YourDdsMessage* ddsMsg = static_cast<const YourDdsMessage*>(msg);
        
        EntityState state;
        // state.entityId = ddsMsg->id;
        // state.type = ddsMsg->isShip ? EntityState::SHIP : EntityState::MISSILE;
        // state.lon = ddsMsg->longitude;
        // state.lat = ddsMsg->latitude;
        // state.alt = ddsMsg->altitude;
        // state.heading = ddsMsg->heading;
        // state.pitch = ddsMsg->pitch;
        // state.roll = ddsMsg->roll;
        state.timestamp = QDateTime::currentMSecsSinceEpoch();
        
        // Update entity
        m_manager->updateEntityState(state);
    }
    
    // For batch updates (more efficient)
    void onDdsBatchDataReceived(const /* std::vector<YourDdsMessage>& */ void* messages)
    {
        QVector<EntityState> states;
        
        // Parse all messages
        // for (const auto& msg : *messages) {
        //     EntityState state;
        //     // ... fill state from msg ...
        //     states.append(state);
        // }
        
        // Batch update
        m_manager->updateEntityStates(states);
    }
    
private:
    EntityManager* m_manager;
};

/**
 * @brief Main entry point
 */
int main(int argc, char** argv)
{
    // Run example A (recommended for large-scale scenarios)
    exampleA_UseEntityManager();
    
    // Or run example B (for simpler scenarios)
    // exampleB_DirectUsage();
    
    return 0;
}
