#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include "VulkanImage.h"

class VulkanDepthResources
{
public:
	VulkanDepthResources();
	~VulkanDepthResources();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, VkExtent2D swapChainExtent);
	void destroy();

	VkImageView getDepthImageView() const;

	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
private:
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkDevice device;
};

