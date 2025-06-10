#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <map>

#include "VulkanUniformBuffers.h"
#include "Renderable.h"
#include "Lights.h"

class VulkanDescriptorSets
{
public:
	VulkanDescriptorSets();
	~VulkanDescriptorSets();

	void create(VkDevice vkdevice, VkDescriptorPool descriptorPool, 
				VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, 
				const std::vector<VkBuffer>& frameUboBuffers,
				const std::vector<VkBuffer>& objectDUBuffers,
				VkImageView textureImageView, VkSampler textureSampler);

	//void createForRenderables(
	//	VkDevice device,
	//	VkDescriptorPool descriptorPool,
	//	VkDescriptorSetLayout descriptorSetLayout,
	//	uint32_t numFrames,
	//	const std::vector<VkBuffer> frameUboBuffers,
	//	const std::vector<VkBuffer> objectDUBuffers,
	//	std::vector<RenderableObject>& renderables
	//);

	void createForMaterials(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t numFrames,
		const std::vector<VkBuffer> frameUboBuffers,
		const std::vector<VkBuffer> objectDUBuffers,
		const std::vector<VkBuffer> lightingUboBuffers,
		std::map<std::string, std::shared_ptr<Material>>& materials
	);

	void destroy();

	VkDescriptorSet getVkDescriptorSet(uint32_t frameIndex) const;

	std::vector<VkDescriptorSet> getVkDescriptorSets() const;

private:
	std::vector<VkDescriptorSet> descriptorSets;

	VkDevice device;
};

