#include "ShipModel.h"
#include <osg/MatrixTransform>

ShipModel::ShipModel(
    double lon,
    double lat,
    double alt,
    double scale,
    const QString& modelPath)
    : Object3D()
{
    // Set initial position and scale
    setPosition(lon, lat, alt);
    setScale(scale);
    setAttitude(0, 0, 0); // Default attitude

    // Load model
    if (!modelPath.isEmpty()) {
        loadModel(modelPath);
    }
}

ShipModel::~ShipModel()
{
    clearSensorVolumes();
}

bool ShipModel::loadModel(const QString& modelPath)
{
    // Load 3D model from file
    m_modelNode = osgDB::readNodeFile(modelPath.toStdString());
    
    if (!m_modelNode.valid()) {
        // If model failed to load, create a simple placeholder (box)
        osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(0, 0, 0), 1000.0);
        osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(box.get());
        drawable->setColor(osg::Vec4(0.5, 0.5, 0.5, 1.0));
        
        osg::ref_ptr<osg::Geode> geode = new osg::Geode();
        geode->addDrawable(drawable.get());
        m_modelNode = geode;
    }
    
    // Add model to the model group
    if (m_modelGroup.valid() && m_modelNode.valid()) {
        // Clear existing children
        m_modelGroup->removeChildren(0, m_modelGroup->getNumChildren());
        m_modelGroup->addChild(m_modelNode.get());
        return true;
    }
    
    return false;
}

void ShipModel::addFixedWave(SensorVolume* sensor)
{
    if (sensor && m_modelGroup.valid()) {
        m_sensorVolumes.push_back(sensor);
        m_modelGroup->addChild(sensor->getGeode());
    }
}

void ShipModel::clearSensorVolumes()
{
    // Remove all sensor volumes from scene graph
    for (auto& sensor : m_sensorVolumes) {
        if (sensor.valid() && m_modelGroup.valid()) {
            m_modelGroup->removeChild(sensor->getGeode());
        }
    }
    m_sensorVolumes.clear();
}

void ShipModel::setSensorVolumesVisible(bool visible)
{
    for (auto& sensor : m_sensorVolumes) {
        if (sensor.valid()) {
            sensor->setVisible(visible);
        }
    }
}

void ShipModel::updateSensorLod(int lodLevel)
{
    for (auto& sensor : m_sensorVolumes) {
        if (sensor.valid()) {
            sensor->setLodLevel(lodLevel);
        }
    }
}
