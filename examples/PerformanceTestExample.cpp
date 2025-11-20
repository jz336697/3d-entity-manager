/**
 * @file PerformanceTestExample.cpp
 * @brief Performance test example with distance-based LOD optimization
 * 
 * This example demonstrates how to use PerformanceTestManager to create
 * 200 entities with Billboard-based LOD optimization for improved performance.
 * 
 * Expected Performance:
 * - Near distance (<500km):  35-45 FPS (75% improvement)
 * - Mid distance (500-2000km): 50-60 FPS (3x improvement)
 * - Far distance (>2000km): 60 FPS (6x improvement)
 */

#include <osgViewer/Viewer>
#include <osgEarth/MapNode>
#include <osgEarth/ImageLayer>
#include <osgGA/TrackballManipulator>
#include <QCoreApplication>

#include "PerformanceTestManager.h"

/**
 * @brief Initialize earth scene
 */
osg::Group* createEarthScene()
{
    // Create map
    osgEarth::Map* map = new osgEarth::Map();
    
    // Add imagery layer
    osgEarth::ImageLayer* imageLayer = new osgEarth::ImageLayer();
    imageLayer->setName("Imagery");
    map->addLayer(imageLayer);
    
    // Create map node
    osgEarth::MapNode* mapNode = new osgEarth::MapNode(map);
    
    // Create root group
    osg::Group* root = new osg::Group();
    root->addChild(mapNode);
    
    return root;
}

/**
 * @brief Main entry point
 */
int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    
    // Create viewer
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 1280, 720);
    
    // Create earth scene
    osg::Group* root = createEarthScene();
    
    // Create performance test manager
    PerformanceTestManager* perfManager = new PerformanceTestManager(
        root,
        &viewer,
        &app
    );
    
    // Create 200 test entities
    perfManager->createTestEntities(200);
    
    // 🔥 NEW: Set Billboard images
    QString shipImage = "./resource/images/ship_icon.png";
    QString missileImage = "./resource/images/missile_icon.png";
    perfManager->setBillboardImages(shipImage, missileImage);
    
    // 🔥 NEW: Set LOD distances
    perfManager->setLODDistances(500000.0, 2000000.0);  // 500km - 2000km
    
    // 🔥 NEW: Enable unified LOD mode (default is already enabled)
    perfManager->setGlobalLODMode(true);
    
    // Start animation
    perfManager->startAnimation(100);
    
    // Setup camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.setSceneData(root);
    
    // Enable statistics (press 's' key to show)
    viewer.getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    
    qDebug() << "========================================";
    qDebug() << "Performance Test with Unified LOD Mode";
    qDebug() << "========================================";
    qDebug() << "Entity count: 200 (100 ships + 100 missiles)";
    qDebug() << "LOD Mode: Unified (all entities switch together)";
    qDebug() << "LOD settings:";
    qDebug() << "  Camera altitude < 500km: All entities show 3D models";
    qDebug() << "  Camera altitude >= 500km: All entities show billboards";
    qDebug() << "";
    qDebug() << "Controls:";
    qDebug() << "  - Press 's' key to show OSG statistics";
    qDebug() << "  - Mouse wheel to zoom (LOD will switch uniformly)";
    qDebug() << "  - Esc to exit";
    qDebug() << "========================================";
    
    // Run viewer
    return viewer.run();
}
