// skybox.vert
#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outUVW;

layout(binding = 0) uniform FrameUbo {
    mat4 view;
    mat4 proj;
} frameData;

void main() {
    outUVW = inPosition;

    mat4 viewNoTranslation = mat4(mat3(frameData.view));
    
    vec4 pos = frameData.proj * viewNoTranslation * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;
}


