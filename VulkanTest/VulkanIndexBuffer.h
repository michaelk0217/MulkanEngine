#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "VulkanBuffer.h"

class VulkanIndexBuffer
{
public:
	VulkanIndexBuffer();
	~VulkanIndexBuffer();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::vector<uint32_t>& indices);
	void destroy();

	VkBuffer getVkBuffer() const;
private:
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkDevice device;
};

