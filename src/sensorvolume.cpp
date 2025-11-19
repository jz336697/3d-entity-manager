#include "sensorvolume.h"
#include <osg/PrimitiveSet>
#include <osg/StateSet>
#include <cmath>

SensorVolume::SensorVolume(
    double radius,
    const osg::Vec4& color,
    double azimuthStart,
    double azimuthEnd,
    double elevationStart,
    double elevationEnd,
    int azimuthStep,
    int elevationStep)
    : m_radius(radius)
    , m_color(color)
    , m_azimuthStart(azimuthStart)
    , m_azimuthEnd(azimuthEnd)
    , m_elevationStart(elevationStart)
    , m_elevationEnd(elevationEnd)
    , m_currentLodLevel(1)
    , m_visible(true)
{
    m_geode = new osg::Geode();
    m_geometry = new osg::Geometry();
    m_geode->addDrawable(m_geometry.get());

    // Setup rendering state for transparency
    osg::StateSet* ss = m_geode->getOrCreateStateSet();
    ss->setMode(GL_BLEND, osg::StateAttribute::ON);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    osg::BlendFunc* bf = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ss->setAttributeAndModes(bf, osg::StateAttribute::ON);
    
    // Disable depth write for proper transparency
    osg::Depth* depth = new osg::Depth();
    depth->setWriteMask(false);
    ss->setAttributeAndModes(depth, osg::StateAttribute::ON);

    // Build initial geometry
    rebuildGeometry();
}

SensorVolume::~SensorVolume()
{
}

void SensorVolume::setLodLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    
    if (m_currentLodLevel != level) {
        m_currentLodLevel = level;
        rebuildGeometry();
    }
}

void SensorVolume::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        m_geode->setNodeMask(visible ? 0xFFFFFFFF : 0x0);
    }
}

void SensorVolume::setRadius(double radius)
{
    if (std::abs(m_radius - radius) > 1.0) {
        m_radius = radius;
        rebuildGeometry();
    }
}

void SensorVolume::setColor(const osg::Vec4& color)
{
    m_color = color;
    rebuildGeometry();
}

void SensorVolume::setAngles(double azimuthStart, double azimuthEnd,
                            double elevationStart, double elevationEnd)
{
    m_azimuthStart = azimuthStart;
    m_azimuthEnd = azimuthEnd;
    m_elevationStart = elevationStart;
    m_elevationEnd = elevationEnd;
    rebuildGeometry();
}

void SensorVolume::rebuildGeometry()
{
    // Determine step sizes based on LOD level
    int azimuthStep, elevationStep;
    switch (m_currentLodLevel) {
        case 0: // High detail
            azimuthStep = LodConfig::SENSOR_AZI_STEP_HIGH;
            elevationStep = LodConfig::SENSOR_ELE_STEP_HIGH;
            break;
        case 1: // Medium detail
            azimuthStep = LodConfig::SENSOR_AZI_STEP_MID;
            elevationStep = LodConfig::SENSOR_ELE_STEP_MID;
            break;
        case 2: // Low detail
        default:
            azimuthStep = LodConfig::SENSOR_AZI_STEP_LOW;
            elevationStep = LodConfig::SENSOR_ELE_STEP_LOW;
            break;
    }

    // Create vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    createVertices(vertices.get(), azimuthStep, elevationStep);

    m_geometry->setVertexArray(vertices.get());

    // Create colors (all vertices same color)
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
    colors->push_back(m_color);
    m_geometry->setColorArray(colors.get());
    m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    // Create triangles
    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES);
    
    int numAziSteps = static_cast<int>((m_azimuthEnd - m_azimuthStart) / azimuthStep) + 1;
    int numEleSteps = static_cast<int>((m_elevationEnd - m_elevationStart) / elevationStep) + 1;

    // Create triangles for the sector surface
    for (int i = 0; i < numAziSteps - 1; ++i) {
        for (int j = 0; j < numEleSteps - 1; ++j) {
            int idx0 = i * numEleSteps + j;
            int idx1 = idx0 + 1;
            int idx2 = (i + 1) * numEleSteps + j;
            int idx3 = idx2 + 1;

            // Triangle 1
            indices->push_back(idx0);
            indices->push_back(idx1);
            indices->push_back(idx2);

            // Triangle 2
            indices->push_back(idx1);
            indices->push_back(idx3);
            indices->push_back(idx2);
        }
    }

    m_geometry->removePrimitiveSet(0, m_geometry->getNumPrimitiveSets());
    m_geometry->addPrimitiveSet(indices.get());
}

void SensorVolume::createVertices(osg::Vec3Array* vertices, 
                                 int azimuthStep, 
                                 int elevationStep)
{
    vertices->clear();

    // Origin point
    osg::Vec3 origin(0, 0, 0);

    // Generate vertices for the sector
    for (double azi = m_azimuthStart; azi <= m_azimuthEnd; azi += azimuthStep) {
        for (double ele = m_elevationStart; ele <= m_elevationEnd; ele += elevationStep) {
            // Convert spherical to Cartesian coordinates
            double aziRad = osg::DegreesToRadians(azi);
            double eleRad = osg::DegreesToRadians(ele);

            double x = m_radius * cos(eleRad) * sin(aziRad);
            double y = m_radius * cos(eleRad) * cos(aziRad);
            double z = m_radius * sin(eleRad);

            vertices->push_back(osg::Vec3(x, y, z));
        }
    }
}
