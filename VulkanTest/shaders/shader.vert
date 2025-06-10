//shader.vert

#version 450

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
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

void main() {
    gl_Position = frameData.proj * frameData.view * objectData.model * vec4(inPosition, 1.0);
    
    // Pass world-space position and normal to fragment shader
    fragPosWorld = vec3(objectData.model * vec4(inPosition, 1.0));
    
    // Calculate the normal matrix to correctly transform normals
    // (avoids issues with non-uniform scaling)
    mat3 normalMatrix = transpose(inverse(mat3(objectData.model)));
    fragNormalWorld = normalize(normalMatrix * inNormal);

    fragTexCoord = inTexCoord;
}