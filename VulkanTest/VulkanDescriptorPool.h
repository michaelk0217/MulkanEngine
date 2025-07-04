#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <array>
#include <vector>
class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool();
	~VulkanDescriptorPool();

	void create(VkDevice vkdevice, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes);
	void destroy();

	VkDescriptorPool getVkDescriptorPool() const;

private:
	VkDescriptorPool descriptorPool;

	VkDevice device;
};

