#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "VulkanUniformBuffers.h"

class VulkanDescriptorSets
{
public:
	VulkanDescriptorSets();
	~VulkanDescriptorSets();

	void create(VkDevice vkdevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, const std::vector<VkBuffer>& uniformBuffers, VkImageView textureImageView, VkSampler textureSampler);
	void destroy();

	VkDescriptorSet getVkDescriptorSet(uint32_t frameIndex) const;

	std::vector<VkDescriptorSet> getVkDescriptorSets() const;

private:
	std::vector<VkDescriptorSet> descriptorSets;

	VkDevice device;
};

