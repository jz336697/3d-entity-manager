#version 120

// Fragment shader for animated track line pulse effect
uniform float pulseTime;
uniform float width;
uniform float speed;
varying float vHeight;

void main()
{
    // Create animated pulse effect based on height and time
    // The sine wave travels along the track line
    float pulse = sin(vHeight / width - pulseTime * speed) * 0.5 + 0.5;
    
    // Apply pulse to alpha channel for animated transparency
    vec4 color = gl_Color;
    color.a *= pulse;
    
    // Output final color
    gl_FragColor = color;
}
