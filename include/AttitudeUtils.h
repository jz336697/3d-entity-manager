#ifndef ATTITUDEUTILS_H
#define ATTITUDEUTILS_H

#include <osg/Quat>
#include <osg/Vec3d>
#include <cmath>

/**
 * @file AttitudeUtils.h
 * @brief Utility functions for attitude calculations
 * 
 * Provides functions for converting between different attitude representations
 * (Euler angles, quaternions) and calculating rotation matrices.
 */

namespace AttitudeUtils {

/**
 * @brief Convert Euler angles (heading, pitch, roll) to quaternion
 * @param heading Heading angle in degrees (yaw, rotation around Z-axis)
 * @param pitch Pitch angle in degrees (rotation around Y-axis)
 * @param roll Roll angle in degrees (rotation around X-axis)
 * @return Quaternion representation
 */
inline osg::Quat eulerToQuat(double heading, double pitch, double roll)
{
    // Convert degrees to radians
    double h = osg::DegreesToRadians(heading);
    double p = osg::DegreesToRadians(pitch);
    double r = osg::DegreesToRadians(roll);
    
    // Create quaternion from Euler angles
    // Order: heading (Z), pitch (Y), roll (X)
    osg::Quat quat;
    quat.makeRotate(
        r, osg::Vec3d(1, 0, 0),  // Roll around X
        p, osg::Vec3d(0, 1, 0),  // Pitch around Y
        h, osg::Vec3d(0, 0, 1)   // Heading around Z
    );
    
    return quat;
}

/**
 * @brief Convert quaternion to Euler angles (heading, pitch, roll)
 * @param quat Quaternion representation
 * @param heading Output heading angle in degrees
 * @param pitch Output pitch angle in degrees
 * @param roll Output roll angle in degrees
 */
inline void quatToEuler(const osg::Quat& quat, double& heading, double& pitch, double& roll)
{
    // Convert quaternion to Euler angles
    double sqw = quat.w() * quat.w();
    double sqx = quat.x() * quat.x();
    double sqy = quat.y() * quat.y();
    double sqz = quat.z() * quat.z();
    
    // Heading (Z-axis rotation)
    heading = atan2(2.0 * (quat.x() * quat.y() + quat.z() * quat.w()), 
                    sqx - sqy - sqz + sqw);
    
    // Pitch (Y-axis rotation)
    pitch = asin(-2.0 * (quat.x() * quat.z() - quat.y() * quat.w()));
    
    // Roll (X-axis rotation)
    roll = atan2(2.0 * (quat.y() * quat.z() + quat.x() * quat.w()), 
                 -sqx - sqy + sqz + sqw);
    
    // Convert radians to degrees
    heading = osg::RadiansToDegrees(heading);
    pitch = osg::RadiansToDegrees(pitch);
    roll = osg::RadiansToDegrees(roll);
}

/**
 * @brief Create a rotation matrix from heading, pitch, roll
 * @param heading Heading angle in degrees
 * @param pitch Pitch angle in degrees
 * @param roll Roll angle in degrees
 * @return Rotation matrix
 */
inline osg::Matrix createRotationMatrix(double heading, double pitch, double roll)
{
    osg::Quat quat = eulerToQuat(heading, pitch, roll);
    return osg::Matrix::rotate(quat);
}

/**
 * @brief Normalize angle to [-180, 180] range
 * @param angle Angle in degrees
 * @return Normalized angle
 */
inline double normalizeAngle(double angle)
{
    while (angle > 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

/**
 * @brief Calculate angular difference between two angles
 * @param angle1 First angle in degrees
 * @param angle2 Second angle in degrees
 * @return Angular difference in degrees [-180, 180]
 */
inline double angleDifference(double angle1, double angle2)
{
    return normalizeAngle(angle2 - angle1);
}

} // namespace AttitudeUtils

#endif // ATTITUDEUTILS_H
