#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include <glm/glm.hpp>

#include "VulkanBuffer.h"


struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class VulkanUniformBuffers
{
public:
	VulkanUniformBuffers();
	~VulkanUniformBuffers();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t numFrames);

	void destroy();

	void update(uint32_t frameIndex, const UniformBufferObject ubo);

	VkBuffer getBuffer(uint32_t frameIndex) const;

	std::vector<VkBuffer> getBuffers() const;

private:
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkDevice device;
	uint32_t frameCount;
};

