#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "VulkanUniformBuffers.h"
#include "Renderable.h"

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

	void createForRenderables(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t numFrames,
		const std::vector<VkBuffer> frameUboBuffers,
		const std::vector<VkBuffer> objectDUBuffers,
		std::vector<RenderableObject>& renderables
	);

	/*void createWithTextureArray(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t numFrames,
		const std::vector<VkBuffer>& frameUboBuffers,
		const std::vector<VkBuffer>& objectDUBuffers,
		const std::vector<std::unique_ptr<VulkanTexture>>& textures
	);*/

	void destroy();

	VkDescriptorSet getVkDescriptorSet(uint32_t frameIndex) const;

	std::vector<VkDescriptorSet> getVkDescriptorSets() const;

private:
	std::vector<VkDescriptorSet> descriptorSets;

	VkDevice device;
};

