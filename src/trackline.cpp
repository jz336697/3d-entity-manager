#include "trackline.h"
#include <osg/PrimitiveSet>
#include <osg/StateSet>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Shader>
#include <osgDB/ReadFile>
#include <cmath>

TrackLine::TrackLine(
    double length,
    double radius,
    const osg::Vec4& color,
    double width,
    double speed,
    int layers)
    : m_length(length)
    , m_radius(radius)
    , m_color(color)
    , m_width(width)
    , m_speed(speed)
    , m_layers(layers)
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
    
    osg::Depth* depth = new osg::Depth();
    depth->setWriteMask(false);
    ss->setAttributeAndModes(depth, osg::StateAttribute::ON);

    // Setup shader for pulse animation
    setupShader();

    // Build initial geometry
    rebuildGeometry();
}

TrackLine::~TrackLine()
{
}

void TrackLine::setLodLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 2) level = 2;
    
    if (m_currentLodLevel != level) {
        m_currentLodLevel = level;
        
        // Update layers based on LOD
        int newLayers;
        switch (level) {
            case 0:
                newLayers = LodConfig::TRACKLINE_LAYERS_HIGH;
                break;
            case 1:
                newLayers = LodConfig::TRACKLINE_LAYERS_MID;
                break;
            case 2:
            default:
                newLayers = LodConfig::TRACKLINE_LAYERS_LOW;
                break;
        }
        
        if (newLayers != m_layers) {
            m_layers = newLayers;
            rebuildGeometry();
        }
    }
}

void TrackLine::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        m_geode->setNodeMask(visible ? 0xFFFFFFFF : 0x0);
    }
}

void TrackLine::setLength(double length)
{
    if (std::abs(m_length - length) > 1.0) {
        m_length = length;
        rebuildGeometry();
    }
}

void TrackLine::setRadius(double radius)
{
    if (std::abs(m_radius - radius) > 0.1) {
        m_radius = radius;
        rebuildGeometry();
    }
}

void TrackLine::setColor(const osg::Vec4& color)
{
    m_color = color;
    rebuildGeometry();
}

void TrackLine::setLayers(int layers)
{
    if (layers != m_layers) {
        m_layers = layers;
        rebuildGeometry();
    }
}

void TrackLine::setPulseTime(float time)
{
    if (m_pulseTimeUniform.valid()) {
        m_pulseTimeUniform->set(time);
    }
}

void TrackLine::setupShader()
{
    // Create shader program
    m_program = new osg::Program();
    
    // Try to load shader files, if they don't exist, use fallback
    osg::Shader* vertShader = osgDB::readShaderFile(osg::Shader::VERTEX, 
        "./resource/osgEarth/trackline_pulse.vert");
    osg::Shader* fragShader = osgDB::readShaderFile(osg::Shader::FRAGMENT,
        "./resource/osgEarth/trackline_pulse.frag");
    
    if (!vertShader) {
        // Fallback vertex shader
        vertShader = new osg::Shader(osg::Shader::VERTEX);
        vertShader->setShaderSource(
            "#version 120\n"
            "uniform float pulseTime;\n"
            "varying float vHeight;\n"
            "void main() {\n"
            "    vHeight = gl_Vertex.z;\n"
            "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
            "    gl_FrontColor = gl_Color;\n"
            "}\n"
        );
    }
    
    if (!fragShader) {
        // Fallback fragment shader with pulse effect
        fragShader = new osg::Shader(osg::Shader::FRAGMENT);
        fragShader->setShaderSource(
            "#version 120\n"
            "uniform float pulseTime;\n"
            "uniform float width;\n"
            "uniform float speed;\n"
            "varying float vHeight;\n"
            "void main() {\n"
            "    float pulse = sin(vHeight / width - pulseTime * speed) * 0.5 + 0.5;\n"
            "    vec4 color = gl_Color;\n"
            "    color.a *= pulse;\n"
            "    gl_FragColor = color;\n"
            "}\n"
        );
    }
    
    m_program->addShader(vertShader);
    m_program->addShader(fragShader);
    
    // Create uniforms
    m_pulseTimeUniform = new osg::Uniform("pulseTime", 0.0f);
    m_widthUniform = new osg::Uniform("width", static_cast<float>(m_width));
    m_speedUniform = new osg::Uniform("speed", static_cast<float>(m_speed));
    
    // Apply to state set
    osg::StateSet* ss = m_geode->getOrCreateStateSet();
    ss->setAttributeAndModes(m_program.get(), osg::StateAttribute::ON);
    ss->addUniform(m_pulseTimeUniform.get());
    ss->addUniform(m_widthUniform.get());
    ss->addUniform(m_speedUniform.get());
}

void TrackLine::rebuildGeometry()
{
    // Create vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
    createVertices(vertices.get(), m_layers);

    m_geometry->setVertexArray(vertices.get());

    // Create colors (all vertices same color)
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
    colors->push_back(m_color);
    m_geometry->setColorArray(colors.get());
    m_geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    // Create triangle strip for the cylinder
    m_geometry->removePrimitiveSet(0, m_geometry->getNumPrimitiveSets());
    m_geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, vertices->size()));
}

void TrackLine::createVertices(osg::Vec3Array* vertices, int layers)
{
    vertices->clear();

    const int segments = 16; // Circle segments around the track line
    const double angleStep = 2.0 * osg::PI / segments;
    const double layerStep = m_length / layers;

    // Create cylindrical track line vertices
    for (int layer = 0; layer <= layers; ++layer) {
        double z = layer * layerStep;
        
        for (int seg = 0; seg <= segments; ++seg) {
            double angle = seg * angleStep;
            double x = m_radius * cos(angle);
            double y = m_radius * sin(angle);
            
            vertices->push_back(osg::Vec3(x, y, z));
        }
    }
}
