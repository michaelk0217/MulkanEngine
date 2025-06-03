#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "VulkanDevice.h"

class VulkanCommandPool
{
public:
	VulkanCommandPool();
	~VulkanCommandPool();

	void create(VkDevice vkdevice, VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vksurface);
	void destroy();

	VkCommandPool getVkCommandPool() const;

private:
	VkCommandPool commandPool;

	VkDevice device;

};

