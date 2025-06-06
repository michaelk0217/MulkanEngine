//shader.vert

#version 450

// layout(binding = 0) uniform UniformBufferObject {
//     mat4 model;
//     mat4 view;
//     mat4 proj;
// } ubo;

layout(binding = 0) uniform FrameUbo {
    mat4 view;
    mat4 proj;
} frameData;

layout(binding = 1) uniform ObjectUbo { // Per-object data
    mat4 model;
} objectData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragColor;

void main() {
    gl_Position = frameData.proj * frameData.view * objectData.model * vec4(inPosition, 1.0);
    // fragColor = inColor;
    fragTexCoord = inTexCoord;
}