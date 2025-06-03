#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

class VulkanSyncObjects
{
public:
	VulkanSyncObjects();
	~VulkanSyncObjects();

	void create(VkDevice vkdevice, uint32_t maxFramesInFlight);
	void destroy();

	VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const;
	VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex) const;
	VkFence getInFlightFence(uint32_t frameIndex) const;

private:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VkDevice device;

	uint32_t maxFrames;
};

