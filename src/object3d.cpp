#include "object3d.h"
#include "AttitudeUtils.h"
#include <osg/Matrix>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <cmath>
#include <QDebug>

// Static member initialization
osg::ref_ptr<osg::EllipsoidModel> Object3D::s_ellipsoid = nullptr;

osg::EllipsoidModel* Object3D::getEllipsoid()
{
    if (!s_ellipsoid.valid()) {
        s_ellipsoid = new osg::EllipsoidModel();
    }
    return s_ellipsoid.get();
}

Object3D::Object3D()
    : m_longitude(0.0)
    , m_latitude(0.0)
    , m_altitude(0.0)
    , m_heading(0.0)
    , m_pitch(0.0)
    , m_roll(0.0)
    , m_scale(1.0)
    , m_visible(true)
    , m_positionDirty(true)
    , m_attitudeDirty(true)
    , m_scaleDirty(true)
{
    // Create scene graph hierarchy with LOD support
    // earth -> lodSwitch -> [0] once -> modelGroup (3D model)
    //                    -> [1] billboardNode (image)
    m_earthTransform = new osg::MatrixTransform();
    m_onceTransform = new osg::MatrixTransform();
    m_modelGroup = new osg::Group();
    m_lodSwitch = new osg::Switch();
    
    m_onceTransform->addChild(m_modelGroup.get());
    m_lodSwitch->addChild(m_onceTransform.get(), true);  // Index 0: 3D model (default: visible)
    m_earthTransform->addChild(m_lodSwitch.get());
}

Object3D::~Object3D()
{
}

void Object3D::setPosition(double lon, double lat, double alt)
{
    // Skip if position hasn't changed significantly (epsilon comparison)
    if (std::abs(m_longitude - lon) < LodConfig::POSITION_EPSILON &&
        std::abs(m_latitude - lat) < LodConfig::POSITION_EPSILON &&
        std::abs(m_altitude - alt) < LodConfig::POSITION_EPSILON) {
        return;
    }
    
    m_longitude = lon;
    m_latitude = lat;
    m_altitude = alt;
    m_positionDirty = true;
}

void Object3D::setPosition(const osg::Vec3d& pos)
{
    setPosition(pos.x(), pos.y(), pos.z());
}

void Object3D::setAttitude(double heading, double pitch, double roll)
{
    // Skip if attitude hasn't changed significantly
    if (std::abs(m_heading - heading) < LodConfig::ATTITUDE_EPSILON &&
        std::abs(m_pitch - pitch) < LodConfig::ATTITUDE_EPSILON &&
        std::abs(m_roll - roll) < LodConfig::ATTITUDE_EPSILON) {
        return;
    }
    
    m_heading = heading;
    m_pitch = pitch;
    m_roll = roll;
    m_attitudeDirty = true;
}

void Object3D::setScale(double scale)
{
    if (std::abs(m_scale - scale) < 1e-6) {
        return;
    }
    
    m_scale = scale;
    m_scaleDirty = true;
}

void Object3D::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        if (m_lodSwitch.valid()) {
            m_lodSwitch->setNodeMask(visible ? ~0u : 0u);
        } else {
            m_earthTransform->setNodeMask(visible ? 0xFFFFFFFF : 0x0);
        }
    }
}

void Object3D::updateIfDirty()
{
    if (m_positionDirty) {
        updateEarthTransform();
        m_positionDirty = false;
    }
    
    if (m_attitudeDirty || m_scaleDirty) {
        updateOnceTransform();
        m_attitudeDirty = false;
        m_scaleDirty = false;
    }
}

void Object3D::updateEarthTransform()
{
    // Convert geodetic coordinates to ECEF (Earth-Centered, Earth-Fixed)
    osg::Vec3d ecef;
    getEllipsoid()->convertLatLongHeightToXYZ(
        osg::DegreesToRadians(m_latitude),
        osg::DegreesToRadians(m_longitude),
        m_altitude,
        ecef.x(), ecef.y(), ecef.z()
    );
    
    // Create local-to-world matrix at this position
    osg::Matrix localToWorld;
    getEllipsoid()->computeLocalToWorldTransformFromXYZ(
        ecef.x(), ecef.y(), ecef.z(), localToWorld
    );
    
    m_earthTransform->setMatrix(localToWorld);
}

void Object3D::updateOnceTransform()
{
    // Create rotation matrix from attitude
    osg::Matrix rotation = AttitudeUtils::createRotationMatrix(m_heading, m_pitch, m_roll);
    
    // Create scale matrix
    osg::Matrix scale = osg::Matrix::scale(m_scale, m_scale, m_scale);
    
    // Combine: scale first, then rotate
    osg::Matrix transform = scale * rotation;
    
    m_onceTransform->setMatrix(transform);
}

void Object3D::createBillboard(const QString& imagePath, double width, double height)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imagePath.toStdString());
    if (!image.valid())
    {
        qWarning() << "[Object3D] Failed to load billboard image:" << imagePath;
        return;
    }

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    osg::ref_ptr<osg::Geometry> quad = osg::createTexturedQuadGeometry(
        osg::Vec3(-width/2, 0, -height/2),
        osg::Vec3(width, 0, 0),
        osg::Vec3(0, 0, height)
    );

    osg::StateSet* ss = quad->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    m_billboardNode = new osg::Billboard();
    m_billboardNode->setMode(osg::Billboard::POINT_ROT_EYE);
    m_billboardNode->addDrawable(quad.get(), osg::Vec3(0, 0, 0));
}

void Object3D::setBillboardImage(const QString& imagePath, double width, double height)
{
    createBillboard(imagePath, width, height);
    
    if (m_billboardNode.valid() && m_lodSwitch.valid())
    {
        if (m_lodSwitch->getNumChildren() < 2)
            m_lodSwitch->addChild(m_billboardNode.get(), false);  // Index 1: image (default: hidden)
        else
            m_lodSwitch->setChild(1, m_billboardNode.get());
    }
}

void Object3D::setLODDistances(double nearDist, double farDist)
{
    m_nearDistance = nearDist;
    // farDist is deprecated - no longer used in two-level LOD strategy
    // Parameter retained for backward compatibility only
}

void Object3D::updateLOD(const osg::Vec3d& eyePosition)
{
    // Skip if auto LOD is disabled
    if (!m_autoLOD)
        return;
    
    if (!m_lodSwitch.valid() || !m_earthTransform.valid())
        return;

    osg::Vec3d objectPos = m_earthTransform->getMatrix().getTrans();
    double distance = (eyePosition - objectPos).length();

    if (distance < m_nearDistance)
    {
        // Near distance: show 3D model
        m_lodSwitch->setValue(0, true);
        m_lodSwitch->setValue(1, false);
    }
    else
    {
        // Far distance: show billboard image (never auto-hide)
        m_lodSwitch->setValue(0, false);
        m_lodSwitch->setValue(1, true);
    }
}

void Object3D::forceLODLevel(int level)
{
    if (!m_lodSwitch.valid())
        return;
    
    if (level == 0)
    {
        // Force show 3D model
        m_lodSwitch->setValue(0, true);   // model on
        m_lodSwitch->setValue(1, false);  // image off
    }
    else if (level == 1)
    {
        // Force show Billboard image
        m_lodSwitch->setValue(0, false);  // model off
        m_lodSwitch->setValue(1, true);   // image on
    }
}

void Object3D::setAutoLOD(bool enabled)
{
    m_autoLOD = enabled;
}
