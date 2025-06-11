// wireframe.frag
#version 450

layout(location = 0) out vec4 outColor;

void main() {
    // Output a solid, bright green color. You can change this to any color you like.
    outColor = vec4(0.0, 1.0, 0.0, 1.0);
}