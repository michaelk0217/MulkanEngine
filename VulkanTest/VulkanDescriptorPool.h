#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <array>
class VulkanDescriptorPool
{
public:
	VulkanDescriptorPool();
	~VulkanDescriptorPool();

	void create(VkDevice vkdevice, uint32_t maxFramesInFlight, uint32_t objectCount);
	void destroy();

	VkDescriptorPool getVkDescriptorPool() const;

private:
	VkDescriptorPool descriptorPool;

	VkDevice device;
};

