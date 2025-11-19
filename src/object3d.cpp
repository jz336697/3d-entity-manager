#include "object3d.h"
#include "AttitudeUtils.h"
#include <osg/Matrix>
#include <cmath>

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
    // Create scene graph hierarchy (optimized - no AutoTransform)
    // earth -> matrix -> once -> modelGroup
    m_earthTransform = new osg::MatrixTransform();
    m_onceTransform = new osg::MatrixTransform();
    m_modelGroup = new osg::Group();
    
    m_earthTransform->addChild(m_onceTransform.get());
    m_onceTransform->addChild(m_modelGroup.get());
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
        m_earthTransform->setNodeMask(visible ? 0xFFFFFFFF : 0x0);
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
