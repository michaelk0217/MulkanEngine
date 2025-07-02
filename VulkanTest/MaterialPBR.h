#pragma once

#include <glm/glm.hpp>

// This struct will be sent to the GPU and defines a material's properties.
struct MaterialUBO {
    alignas(16) glm::vec4 baseColorFactor;
    alignas(16) glm::vec4 emissiveFactor;

    alignas(4) float metallicFactor;
    alignas(4) float roughnessFactor;

    // Bools (as ints) to tell the shader which maps to sample
    alignas(4) int hasAlbedoMap;
    alignas(4) int hasNormalMap;
    alignas(4) int hasMetallicRoughnessMap;
    alignas(4) int hasOcclusionMap;
    alignas(4) int hasEmissiveMap;

    // Add padding to ensure the total size is a multiple of 16 for std140 layout
    alignas(4) float padding1;
    alignas(4) int padding2;
    alignas(4) int padding3;
};