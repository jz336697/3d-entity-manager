#version 120

// Vertex shader for animated track line pulse effect
uniform float pulseTime;
varying float vHeight;

void main()
{
    // Pass vertex height to fragment shader
    vHeight = gl_Vertex.z;
    
    // Transform vertex to clip space
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // Pass through color
    gl_FrontColor = gl_Color;
}
