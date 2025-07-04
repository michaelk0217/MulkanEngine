#pragma once
// MaterialPBR.h
#include <glm/glm.hpp>

struct MaterialUBO {
    alignas(16) glm::vec4 baseColorFactor;
    alignas(16) glm::vec4 emissiveFactor;

    alignas(4) float metallicFactor;
    alignas(4) float roughnessFactor;

    alignas(4) int hasAlbedoMap;
    alignas(4) int hasNormalMap;
    alignas(4) int hasMetallicRoughnessMap;
    alignas(4) int hasOcclusionMap;
    alignas(4) int hasEmissiveMap;

    alignas(4) float padding1;
    alignas(4) int padding2;
    alignas(4) int padding3;
};