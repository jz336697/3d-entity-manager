#ifndef DDSDATASIMULATOR_H
#define DDSDATASIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <cmath>
#include "EntityManager.h"

/**
 * @file DdsDataSimulator.h
 * @brief DDS data simulator for testing without real DDS
 * 
 * Simulates entity movement and state updates for testing purposes.
 * In production, replace this with real DDS message handlers.
 */

class DdsDataSimulator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param entityManager Entity manager to update
     * @param parent Qt parent object
     */
    explicit DdsDataSimulator(EntityManager* entityManager, QObject* parent = nullptr);
    virtual ~DdsDataSimulator();

    /**
     * @brief Start simulation
     * @param updateIntervalMs Update interval in milliseconds
     */
    void start(int updateIntervalMs = 100);

    /**
     * @brief Stop simulation
     */
    void stop();

    /**
     * @brief Set simulation parameters
     * @param speedFactor Movement speed multiplier (default 1.0)
     * @param altitudeVariation Altitude variation range (default 50000m)
     */
    void setSimulationParams(double speedFactor = 1.0, double altitudeVariation = 50000.0);

public slots:
    /**
     * @brief Update simulation step
     */
    void updateSimulation();

protected:
    /**
     * @brief Generate simulated entity state
     * @param entityId Entity identifier
     * @param type Entity type
     * @param time Simulation time
     * @return Simulated entity state
     */
    EntityState generateEntityState(int entityId, EntityState::Type type, double time);

private:
    EntityManager* m_entityManager;
    QTimer* m_timer;
    
    double m_simulationTime;
    double m_speedFactor;
    double m_altitudeVariation;
};

#endif // DDSDATASIMULATOR_H
