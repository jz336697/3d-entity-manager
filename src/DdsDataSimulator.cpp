#include "DdsDataSimulator.h"
#include "EntityManager.h"
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

DdsDataSimulator::DdsDataSimulator(EntityManager* entityManager, QObject* parent)
    : QObject(parent),
    m_entityManager(entityManager),
    m_timer(new QTimer(this)),
    m_movementSpeed(50.0),      // 默认50米/秒
    m_rotationSpeed(10.0),      // 默认10度/秒
    m_lastUpdateTime(0)
{
    connect(m_timer, &QTimer::timeout, this, &DdsDataSimulator::onTimeout);
    initializeEntities();
}

DdsDataSimulator::~DdsDataSimulator()
{
    stop();
}

void DdsDataSimulator::start(int intervalMs)
{
    if (m_timer->isActive()) {
        qWarning() << "[DdsDataSimulator] Already running";
        return;
    }

    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    m_timer->start(intervalMs);
    qDebug() << "[DdsDataSimulator] Started with interval:" << intervalMs << "ms";
}

void DdsDataSimulator::stop()
{
    if (m_timer->isActive()) {
        m_timer->stop();
        qDebug() << "[DdsDataSimulator] Stopped";
    }
}

void DdsDataSimulator::setUpdateInterval(int ms)
{
    if (m_timer->isActive()) {
        m_timer->setInterval(ms);
    }
}

void DdsDataSimulator::setMovementSpeed(double speed)
{
    m_movementSpeed = speed;
}

void DdsDataSimulator::setRotationSpeed(double speed)
{
    m_rotationSpeed = speed;
}

void DdsDataSimulator::initializeEntities()
{
    // 从 EntityManager 获取已创建的实体列表
    if (!m_entityManager) {
        qWarning() << "[DdsDataSimulator] EntityManager is null";
        return;
    }

    // 假设 EntityManager 有获取实体ID列表的方法
    // 这里使用示例数据
    QVector<int> entityIds;
    for (int i = 0; i < 200; ++i) {
        entityIds.append(i);
    }

    // 初始化每个实体的状态和运动参数
    for (int id : entityIds) {
        EntityState state;
        state.entityId = id;
        state.type = (id % 2 == 0) ? EntityState::SHIP : EntityState::MISSILE;

        // 随机初始位置（中国东海附近）
        state.lon = 120.0 + QRandomGenerator::global()->bounded(10.0);  // 120-130度
        state.lat = 25.0 + QRandomGenerator::global()->bounded(10.0);   // 25-35度
        state.alt = (state.type == EntityState::SHIP) ? 0.0 : 10000.0;  // 船在海平面，导弹在高空

        // 随机姿态
        state.heading = QRandomGenerator::global()->bounded(360.0);
        state.pitch = QRandomGenerator::global()->bounded(-10.0, 10.0);
        state.roll = QRandomGenerator::global()->bounded(-5.0, 5.0);

        state.timestamp = QDateTime::currentMSecsSinceEpoch();

        m_entityStates.append(state);

        // 初始化运动参数（圆周运动）
        MotionParams motion;
        motion.circleRadius = 0.5 + QRandomGenerator::global()->bounded(2.0);  // 0.5-2.5度半径
        motion.circleAngle = QRandomGenerator::global()->bounded(360.0);
        motion.centerLon = state.lon;
        motion.centerLat = state.lat;
        motion.angularVelocity = (QRandomGenerator::global()->bounded(2) == 0) ? 5.0 : -5.0;  // 顺时针或逆时针

        // 随机速度向量
        double angle = QRandomGenerator::global()->bounded(360.0);
        double speed = 20.0 + QRandomGenerator::global()->bounded(80.0);  // 20-100米/秒
        motion.vx = speed * qCos(qDegreesToRadians(angle));
        motion.vy = speed * qSin(qDegreesToRadians(angle));
        motion.vz = (state.type == EntityState::MISSILE)
                        ? QRandomGenerator::global()->bounded(-10.0, 10.0)  // 导弹有垂直速度
                        : 0.0;

        m_motionParams[id] = motion;
    }

    qDebug() << "[DdsDataSimulator] Initialized" << m_entityStates.size() << "entities";
}

void DdsDataSimulator::onTimeout()
{
    updateEntityStates();

    // 发送批量数据到 EntityManager
    if (m_entityManager) {
        m_entityManager->updateEntityStates(m_entityStates);
    }

    // 发送信号
    emit batchDataGenerated(m_entityStates);
}

void DdsDataSimulator::updateEntityStates()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double deltaTime = (now - m_lastUpdateTime) / 1000.0;  // 转换为秒
    m_lastUpdateTime = now;

    if (deltaTime > 1.0) {
        deltaTime = 0.1;  // 防止第一次更新时间差过大
    }

    for (int i = 0; i < m_entityStates.size(); ++i) {
        EntityState& state = m_entityStates[i];
        MotionParams& motion = m_motionParams[state.entityId];

        // 圆周运动
        motion.circleAngle += motion.angularVelocity * deltaTime;
        if (motion.circleAngle > 360.0) motion.circleAngle -= 360.0;
        if (motion.circleAngle < 0.0) motion.circleAngle += 360.0;

        // 更新位置（圆周运动）
        double angleRad = qDegreesToRadians(motion.circleAngle);
        state.lon = motion.centerLon + motion.circleRadius * qCos(angleRad);
        state.lat = motion.centerLat + motion.circleRadius * qSin(angleRad);

        // 导弹高度变化
        if (state.type == EntityState::MISSILE) {
            state.alt += motion.vz * deltaTime;

            // 限制高度范围
            if (state.alt < 1000.0) {
                state.alt = 1000.0;
                motion.vz = qAbs(motion.vz);  // 反弹
            } else if (state.alt > 50000.0) {
                state.alt = 50000.0;
                motion.vz = -qAbs(motion.vz);  // 反弹
            }
        }

        // 更新航向（根据运动方向）
        double dx = qCos(angleRad);
        double dy = qSin(angleRad);
        state.heading = qRadiansToDegrees(qAtan2(dy, dx));
        if (state.heading < 0) state.heading += 360.0;

        // 轻微的俯仰和滚转变化
        state.pitch += QRandomGenerator::global()->bounded(-1.0, 1.0) * deltaTime;
        state.pitch = qBound(-15.0, state.pitch, 15.0);

        state.roll += QRandomGenerator::global()->bounded(-2.0, 2.0) * deltaTime;
        state.roll = qBound(-20.0, state.roll, 20.0);

        // 速度
        state.speedX = motion.vx;
        state.speedY = motion.vy;
        state.speedZ = motion.vz;

        // 时间戳
        state.timestamp = now;
    }
}

EntityState DdsDataSimulator::generateRandomState(int entityId, EntityState::Type type)
{
    EntityState state;
    state.entityId = entityId;
    state.type = type;

    // 随机位置
    state.lon = 120.0 + QRandomGenerator::global()->bounded(10.0);
    state.lat = 25.0 + QRandomGenerator::global()->bounded(10.0);
    state.alt = (type == EntityState::SHIP) ? 0.0 : 10000.0;

    // 随机姿态
    state.heading = QRandomGenerator::global()->bounded(360.0);
    state.pitch = QRandomGenerator::global()->bounded(-10.0, 10.0);
    state.roll = QRandomGenerator::global()->bounded(-5.0, 5.0);

    state.timestamp = QDateTime::currentMSecsSinceEpoch();

    return state;
}
