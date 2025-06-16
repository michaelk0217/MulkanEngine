// skybox.frag
#version 450

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform samplerCube skyboxSampler;

void main() {
    outColor = texture(skyboxSampler, inUVW);
}