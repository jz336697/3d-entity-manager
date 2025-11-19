#ifndef MISSILEMODEL_H
#define MISSILEMODEL_H

#include "object3d.h"
#include "trackline.h"
#include <osgDB/ReadFile>
#include <QString>
#include <QVector>

/**
 * @file MissileModel.h
 * @brief Missile entity model with track lines
 * 
 * Represents a missile entity with 3D model and optional track lines.
 * Inherits optimized dirty flag system from Object3D.
 */

class MissileModel : public Object3D
{
public:
    /**
     * @brief Constructor
     * @param lon Longitude in degrees
     * @param lat Latitude in degrees
     * @param alt Altitude in meters
     * @param heading Heading angle in degrees
     * @param pitch Pitch angle in degrees
     * @param roll Roll angle in degrees
     * @param scale Model scale factor
     * @param modelPath Path to 3D model file
     */
    MissileModel(
        double lon,
        double lat,
        double alt,
        double heading,
        double pitch,
        double roll,
        double scale,
        const QString& modelPath
    );

    virtual ~MissileModel();

    /**
     * @brief Load 3D model from file
     * @param modelPath Path to model file
     * @return true if successful
     */
    bool loadModel(const QString& modelPath);

    /**
     * @brief Add a radar track line pointing to a target
     * @param trackLine Track line to add
     * @param targetNode Target node to point to (optional)
     */
    void addRadarTrackLine(TrackLine* trackLine, osg::Node* targetNode = nullptr);

    /**
     * @brief Remove all track lines
     */
    void clearTrackLines();

    /**
     * @brief Set visibility of all track lines
     */
    void setTrackLinesVisible(bool visible);

    /**
     * @brief Update track line LOD based on distance
     * @param lodLevel LOD level (0=high, 1=mid, 2=low)
     */
    void updateTrackLineLod(int lodLevel);

    /**
     * @brief Get all track lines
     */
    const QVector<osg::ref_ptr<TrackLine>>& getTrackLines() const {
        return m_trackLines;
    }

protected:
    // Model node
    osg::ref_ptr<osg::Node> m_modelNode;
    
    // Track lines attached to this missile
    QVector<osg::ref_ptr<TrackLine>> m_trackLines;
    
    // Track line offset from model origin (to start from missile tip)
    osg::Vec3 m_trackLineOffset;
};

#endif // MISSILEMODEL_H
