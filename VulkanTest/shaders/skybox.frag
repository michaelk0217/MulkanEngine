// skybox.frag
#version 450

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform samplerCube skyboxSampler;

void main() {
    outColor = texture(skyboxSampler, inUVW);
}

// layout(location = 0) in vec3 inUVW;

// layout(location = 0) out vec4 outColor;

// void main()
// {
//     // Bypass the texture/sampler and output a solid, bright color like cyan.
//     outColor = vec4(0.0, 1.0, 1.0, 1.0);
// }