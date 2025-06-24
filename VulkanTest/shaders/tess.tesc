// tess.tesc
// Tessellation Control Shader
#version 450

layout(vertices = 3) out;

layout(location = 0) in vec3 inNormal[];
layout(location = 1) in vec2 inTexCoord[];

// outputs to tessellation evaluation shader
layout(location = 0) out vec3 outNormal_TES[];
layout(location = 1) out vec2 outTexCoord_TES[];

layout(binding = 10) uniform TessellationUBO {
    float tessellationLevel;
    float displacementScale;
} tessUbo;

void main()
{
    // --- Per-Vertex work (runs for each of the 3 vertices) ---
    // Pass control point attributes to the evaluation shader.
    outNormal_TES[gl_InvocationID] = inNormal[gl_InvocationID];
    outTexCoord_TES[gl_InvocationID] = inTexCoord[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // --- Per-Patch work (runs only once for the whole patch) ---
    // According to the spec, this should only be done in one invocation.
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tessUbo.tessellationLevel;
        gl_TessLevelOuter[1] = tessUbo.tessellationLevel;
        gl_TessLevelOuter[2] = tessUbo.tessellationLevel;
        gl_TessLevelInner[0] = tessUbo.tessellationLevel;
    }
}