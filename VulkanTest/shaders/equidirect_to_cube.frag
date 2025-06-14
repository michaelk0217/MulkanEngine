// equirect_to_cube.frag
#version 450

layout (location = 0) in vec3 inLocalPos;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2*PI), 1 / PI

// Converts a 3D direction vector to a 2D UV coordinate on an equirectangular map
vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = sampleSphericalMap(normalize(inLocalPos));
    outColor = texture(equirectangularMap, uv);
}


// #version 450

// layout (location = 0) out vec4 outColor;

// // input variables are now unused

// void main()
// {
//     // Completely ignore the HDR texture and just output solid red.
//     outColor = vec4(1.0, 0.0, 0.0, 1.0); 
// }