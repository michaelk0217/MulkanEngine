#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "VulkanBuffer.h"
#include "ModelLoader.h"

class VulkanVertexBuffer
{
public:
	VulkanVertexBuffer();
	~VulkanVertexBuffer();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::vector<Vertex>& vertices);
	void destroy();

	VkBuffer getVkBuffer() const;

private:
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkDevice device;
};

