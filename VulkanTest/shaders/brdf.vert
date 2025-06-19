#version 450

layout(location = 0) out vec2 outUV;

void main()
{
    // The UV coordinates are generated based on the vertex index.
    // This maps the vertices of our giant triangle to the corners of a quad.
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    
    // By manipulating the outUV coords, we generate positions that cover the screen.
    // This creates a triangle that spans from (-1,-1) to (3,3) in clip space.
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0, 1.0);
}