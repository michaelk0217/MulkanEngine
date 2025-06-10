#pragma once

#include <memory>
#include <string>
#include <vector>
#include "VulkanTexture.h"
#include "VulkanGlobals.h"

class VulkanTexture;

struct Material
{
	// A descriptive name for the material
	std::string name;

	// Pointers to the textures that define the surface.
	// We use shared_ptr because multiple objects might share the same material.
	std::shared_ptr<VulkanTexture> albedoMap; // Base color
	std::shared_ptr<VulkanTexture> normalMap; // Surface detail
	//std::shared_ptr<VulkanTexture> metallicRoughnessMap; // Metalness (R channel)
	//std::shared_ptr<VulkanTexture> aoMap; // Ambient Occlusion
	std::shared_ptr<VulkanTexture> ormMap; // R: Occlusion, G: Roughness, B: Metallic

	std::vector<VkDescriptorSet> frameSpecificDescriptorSets;

	Material() : frameSpecificDescriptorSets(VulkanGlobals::MAX_FRAMES_IN_FLIGHT) {}
};