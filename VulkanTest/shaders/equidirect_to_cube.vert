// equirect_to_cube.vert
// #version 450

// layout (location = 0) in vec3 inPos;

// layout (location = 0) out vec3 outLocalPos;

// // Pass the view and projection matrix for the current face being rendered.
// // A Push Constant is efficient for this.
// layout(push_constant) uniform PushBlock {
//     mat4 mvp;
// } pushBlock;

// void main() {
//     outLocalPos = inPos;
//     gl_Position = pushBlock.mvp * vec4(inPos, 1.0);
// }

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