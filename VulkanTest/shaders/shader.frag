//shader.frag (frag.spv)

#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inFragPosWorld;
layout(location = 2) in vec3 inNormalWorld;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform SceneLightingUBO
{
    vec4 lightDir;
    vec4 lightColor;
    vec4 viewPos;
} sceneUbo;

// PBR SAMPLERS
layout(binding = 3) uniform sampler2D albedoMap;
layout(binding = 4) uniform sampler2D normalMap;
layout(binding = 5) uniform sampler2D ormMap;

// IBL SAMPLERS
layout(binding = 8) uniform samplerCube irradianceMap;
layout(binding = 9) uniform samplerCube prefilterMap;
layout(binding = 10) uniform sampler2D brdfLut;


const float PI = 3.14159265359;

// --- PBR Functions (from LearnOpenGL PBR) ---
// Normal Distribution function
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry Function
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel Equation
vec3 fresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // --- Material Property Setup ---
    vec3 albedo = texture(albedoMap, inTexCoord).rgb;
    vec3 ormData = texture(ormMap, inTexCoord).rgb;
    float ao = ormData.r;
    float roughness = ormData.g;
    float metallic = ormData.b;
    vec3 N = normalize(inNormalWorld); // The surface normal
    vec3 V = normalize(sceneUbo.viewPos.xyz - inFragPosWorld); // The view vector

    // Calculate F0 (Base reflectivity) for frensel equation
    // Dielectrics (non-metals) have a base reflectivity of about 4%,
    // while metals use their albedo color as their base reflectivity.
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // --- Start Direct Lighting Calculation ---
    // (loop if multiple lights)
    // for Lo is our outgoing radiance for single light
    vec3 Lo = vec3(0.0);
    vec3 L = normalize(-sceneUbo.lightDir.xyz);
    vec3 H = normalize(V + L);
    vec3 radiance = sceneUbo.lightColor.rgb * sceneUbo.lightColor.w;
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F_direct = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 numerator = NDF * G * F_direct;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Add small epsilon to prevent division by zero.
    vec3 specular_direct = numerator / denominator;
    // Non-Metals (metallic = 0) reflect all lights diffusely.
    vec3 kD_direct = vec3(1.0) - F_direct;
    kD_direct *= 1.0 - metallic;
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD_direct * albedo / PI + specular_direct) * radiance * NdotL;
    // --- End Direct Lighting Calculation ---

    // --- Start Indirect Lighting Calculation ---
    vec3 F_indirect = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kS = F_indirect;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic; // Non-metals have no metallic reflections
    // Indirect Diffuse (from Irradiance Map)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse_indirect = irradiance * albedo;
    // 2. Indirect Specular (from Prefilter Map and BRDF LUT)
    vec3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 4.0; // Should match number of mips - 1
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular_indirect = prefilteredColor * (F_indirect * brdf.x + brdf.y);

    // Calculate Ambient Lighting
    // -- Simple Approximation for indirect light, modulated by the Ambient Occulsion map.
    vec3 ambient = (kD * diffuse_indirect + specular_indirect) * ao;

    // Combining direct lighting (Lo) and indirect/ambient lighting
    vec3 color = ambient + Lo;

    // Tone Mapping & Gamma Correction
    color = color / (color + vec3(1.0)); // Basic Reinhard tone mapping
    // color = pow(color, vec3(1.0/2.2)); // Apply gamma correction

    outColor = vec4(color, 1.0);
}