// tess.tese
// tessellation evaluation

#version 450

// Define the input primitive from the tessellator
layout(triangles, equal_spacing, cw) in;

layout(binding = 0) uniform FrameUbo {
    mat4 view;
    mat4 proj;
} frameData; // The instance name is 'frameData'

layout(binding = 1) uniform ObjectUbo { // Per-object data
    mat4 model;
} objectData; // The instance name is 'objectData'

layout(binding = 9) uniform sampler2D displacementMap;

layout(binding = 10) uniform TessellationUBO {
    float tessellationLevel;
    float displacementScale;
} tessUbo;

// Inputs from TCS (for the 3 control points of the patch)
layout(location = 0) in vec3 inNormal_TES[];
layout(location = 1) in vec2 inTexCoord_TES[];

// outputs to fragment shader
layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

void main()
{
    // Interpolate per-vertex attributes across the patch
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 pos = gl_TessCoord.x * p0 + gl_TessCoord.y * p1 + gl_TessCoord.z * p2;

    vec2 uv0 = inTexCoord_TES[0];
    vec2 uv1 = inTexCoord_TES[1];
    vec2 uv2 = inTexCoord_TES[2];
    fragTexCoord = gl_TessCoord.x * uv0 + gl_TessCoord.y * uv1 + gl_TessCoord.z * uv2;

    vec3 n0 = inNormal_TES[0];
    vec3 n1 = inNormal_TES[1];
    vec3 n2 = inNormal_TES[2];
    vec3 normal = normalize(gl_TessCoord.x * n0 + gl_TessCoord.y * n1 + gl_TessCoord.z * n2);

    // sample displacement map
    float displacement = texture(displacementMap, fragTexCoord).r;

    // Displace the vertex position along its normal
    pos.xyz += normal * displacement * tessUbo.displacementScale;

    // transform to world and clip space
    fragPosWorld = vec3(objectData.model * vec4(pos, 1.0)); // FIX: Use 'objectData'
    mat3 normalMatrix = transpose(inverse(mat3(objectData.model))); // FIX: Use 'objectData'
    fragNormalWorld = normalize(normalMatrix * normal);

    gl_Position = frameData.proj * frameData.view * vec4(fragPosWorld, 1.0); // FIX: Use 'frameData'
}