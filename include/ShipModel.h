#ifndef SHIPMODEL_H
#define SHIPMODEL_H

#include "object3d.h"
#include "sensorvolume.h"
#include <osgDB/ReadFile>
#include <QString>
#include <QVector>

/**
 * @file ShipModel.h
 * @brief Ship entity model with sensor volumes
 * 
 * Represents a ship entity with 3D model and optional sensor coverage volumes.
 * Inherits optimized dirty flag system from Object3D.
 */

class ShipModel : public Object3D
{
public:
    /**
     * @brief Constructor
     * @param lon Longitude in degrees
     * @param lat Latitude in degrees
     * @param alt Altitude in meters
     * @param scale Model scale factor
     * @param modelPath Path to 3D model file (.osgb, .osg, .obj, etc.)
     */
    ShipModel(
        double lon,
        double lat,
        double alt,
        double scale,
        const QString& modelPath
    );

    virtual ~ShipModel();

    /**
     * @brief Load 3D model from file
     * @param modelPath Path to model file
     * @return true if successful
     */
    bool loadModel(const QString& modelPath);

    /**
     * @brief Add a fixed sensor volume (e.g., radar coverage)
     * The sensor volume will be attached to the ship and move with it
     * @param sensor Sensor volume to add
     */
    void addFixedWave(SensorVolume* sensor);

    /**
     * @brief Remove all sensor volumes
     */
    void clearSensorVolumes();

    /**
     * @brief Set visibility of all sensor volumes
     */
    void setSensorVolumesVisible(bool visible);

    /**
     * @brief Update sensor volume LOD based on distance
     * @param lodLevel LOD level (0=high, 1=mid, 2=low)
     */
    void updateSensorLod(int lodLevel);

    /**
     * @brief Get all sensor volumes
     */
    const QVector<osg::ref_ptr<SensorVolume>>& getSensorVolumes() const {
        return m_sensorVolumes;
    }

protected:
    // Model node
    osg::ref_ptr<osg::Node> m_modelNode;
    
    // Sensor volumes attached to this ship
    QVector<osg::ref_ptr<SensorVolume>> m_sensorVolumes;
};

#endif // SHIPMODEL_H
