#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

#include <glm/glm.hpp>

#include "VulkanGlobals.h"
#include "VulkanBuffer.h"
#include "Lights.h"


struct FrameUniformBufferObject 
{
	//alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	// Later add lighting global data like camera position, ambietn light, etc
};

struct ObjectUniformBufferObject 
{
	alignas(16) glm::mat4 model;
	// Later could add material IDs or other per-object shader params
};

class VulkanUniformBuffers
{
public:
	VulkanUniformBuffers();
	~VulkanUniformBuffers();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t numFrames, 
		VkDeviceSize totalBufferSize = sizeof(FrameUniformBufferObject),
		bool isDynamic = false);

	void destroy();

	void update(uint32_t frameIndex, const FrameUniformBufferObject ubo);

	void updateDynamic(uint32_t frameIndex, uint32_t objectIndex, const ObjectUniformBufferObject& ubo);

	void updateLights(uint32_t frameIndex, SceneLightingUBO& lightUbo);

	VkBuffer getBuffer(uint32_t frameIndex) const;

	std::vector<VkBuffer> getBuffers() const;

	static VkDeviceSize totalObjectDataBufferSize(VkPhysicalDevice physdevice);

	VkDeviceSize getDynamicAlignment() const { return dynamicAlignment; }

private:
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkDevice device;
	uint32_t frameCount;

	bool isDynamic;
	VkDeviceSize dynamicAlignment;
};

