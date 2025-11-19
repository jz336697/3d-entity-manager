#include "MissileModel.h"
#include <osg/MatrixTransform>

MissileModel::MissileModel(
    double lon,
    double lat,
    double alt,
    double heading,
    double pitch,
    double roll,
    double scale,
    const QString& modelPath)
    : Object3D()
    , m_trackLineOffset(0, 0, 0)
{
    // Set initial position, attitude and scale
    setPosition(lon, lat, alt);
    setAttitude(heading, pitch, roll);
    setScale(scale);

    // Load model
    if (!modelPath.isEmpty()) {
        loadModel(modelPath);
    }
}

MissileModel::~MissileModel()
{
    clearTrackLines();
}

bool MissileModel::loadModel(const QString& modelPath)
{
    // Load 3D model from file
    m_modelNode = osgDB::readNodeFile(modelPath.toStdString());
    
    if (!m_modelNode.valid()) {
        // If model failed to load, create a simple placeholder (cone)
        osg::ref_ptr<osg::Cone> cone = new osg::Cone(osg::Vec3(0, 0, 0), 200.0, 1000.0);
        osg::ref_ptr<osg::ShapeDrawable> drawable = new osg::ShapeDrawable(cone.get());
        drawable->setColor(osg::Vec4(1.0, 0.5, 0.0, 1.0));
        
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

void MissileModel::addRadarTrackLine(TrackLine* trackLine, osg::Node* targetNode)
{
    if (trackLine && m_modelGroup.valid()) {
        m_trackLines.push_back(trackLine);
        
        // Create a transform for the track line offset
        osg::ref_ptr<osg::MatrixTransform> offsetTransform = new osg::MatrixTransform();
        offsetTransform->setMatrix(osg::Matrix::translate(m_trackLineOffset));
        offsetTransform->addChild(trackLine->getGeode());
        
        m_modelGroup->addChild(offsetTransform.get());
        
        // If a target node is specified, you could add logic here to orient
        // the track line towards it (requires additional transform calculations)
        // For now, track line extends in +Z direction from missile
    }
}

void MissileModel::clearTrackLines()
{
    // Remove all track lines from scene graph
    // Note: Since track lines are added through offset transforms,
    // we need to find and remove those transforms
    if (m_modelGroup.valid()) {
        // Remove all children except the model node
        unsigned int numChildren = m_modelGroup->getNumChildren();
        for (unsigned int i = numChildren; i > 0; --i) {
            osg::Node* child = m_modelGroup->getChild(i - 1);
            if (child != m_modelNode.get()) {
                m_modelGroup->removeChild(i - 1);
            }
        }
    }
    
    m_trackLines.clear();
}

void MissileModel::setTrackLinesVisible(bool visible)
{
    for (auto& trackLine : m_trackLines) {
        if (trackLine.valid()) {
            trackLine->setVisible(visible);
        }
    }
}

void MissileModel::updateTrackLineLod(int lodLevel)
{
    for (auto& trackLine : m_trackLines) {
        if (trackLine.valid()) {
            trackLine->setLodLevel(lodLevel);
        }
    }
}
