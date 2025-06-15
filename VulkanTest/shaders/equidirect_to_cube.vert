// equirect_to_cube.vert
#version 450

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outLocalPos;

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pushConstants;

void main() {
    outLocalPos = inPosition;
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
}