#pragma once
// Material.h
#include <memory>
#include <string>
#include <vector>
#include "VulkanTexture.h"
#include "VulkanGlobals.h"
#include "MaterialPBR.h"
class VulkanTexture;

//struct Material
//{
//	std::string name;
//	// Pointers to the textures that define the surface.
//	std::shared_ptr<VulkanTexture> albedoMap; // Base color
//	std::shared_ptr<VulkanTexture> normalMap; // Surface detail
//	std::shared_ptr<VulkanTexture> ormMap; // R: Occlusion, G: Roughness, B: Metallic
//
//	std::shared_ptr<VulkanTexture> aoMap;
//	std::shared_ptr<VulkanTexture> roughnessMap;
//	std::shared_ptr<VulkanTexture> metallnessMap;
//
//	std::shared_ptr<VulkanTexture> displacementMap;
//
//	std::shared_ptr<VulkanTexture> emissiveMap;;
//	bool useOrm = true;
//
//	std::vector<VkDescriptorSet> frameSpecificDescriptorSets;
//
//	Material() : frameSpecificDescriptorSets(VulkanGlobals::MAX_FRAMES_IN_FLIGHT) {}
//
//	// glTF Material Properties
//	glm::vec4 baseColorFactor = glm::vec4(1.0f);
//	float metallicFactor = 1.0f;
//	float roughnessFactor = 1.0f;
//	float normalScale = 1.0f;
//	float occlusioinStrength = 1.0f;
//	glm::vec3 emissiveFactor = glm::vec3(0.0f);
//
//	// Alpha properties
//	enum class AlphaMode { OPAQUE_MODE, MASK_MODE, BLEND_MODE };
//	AlphaMode alphaMode = AlphaMode::OPAQUE_MODE;
//	float alphaCutoff = 0.5f;
//
//	// Double sided rendering
//	bool doubleSided = false;
//
//	std::string gltfSourceFile;
//	int gltfMaterialIndex = -1;
//};
struct Material
{
	std::string name;
	MaterialUBO uboData{};

	std::shared_ptr<VulkanTexture> albedoMap;
	std::shared_ptr<VulkanTexture> normalMap;
	std::shared_ptr<VulkanTexture> metallicRoughnessMap; // Use one map for ORM or just metallic/roughness.
	std::shared_ptr<VulkanTexture> occlusionMap;
	std::shared_ptr<VulkanTexture> emissiveMap;

	std::vector<VkDescriptorSet> frameSpecificDescriptorSets;
	bool doubleSided = false;

	Material() : frameSpecificDescriptorSets(VulkanGlobals::MAX_FRAMES_IN_FLIGHT) {}

};