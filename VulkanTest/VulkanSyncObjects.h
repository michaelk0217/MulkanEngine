#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

class VulkanSyncObjects
{
public:
	VulkanSyncObjects();
	~VulkanSyncObjects();

	void create(VkDevice vkdevice, uint32_t maxFramesInFlight, uint32_t numSwapchainImages);
	void destroy();

	VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const;
	VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex) const;
	VkFence getInFlightFence(uint32_t frameIndex) const;

	VkFence& getImageInFlightFence(uint32_t imageIndex);

private:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	std::vector<VkFence> imagesInFlight;

	VkDevice device;

	uint32_t maxFrames;
};

