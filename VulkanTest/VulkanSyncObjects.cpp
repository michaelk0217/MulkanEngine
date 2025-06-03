#include "VulkanSyncObjects.h"

VulkanSyncObjects::VulkanSyncObjects() : imageAvailableSemaphores({}), renderFinishedSemaphores({}), inFlightFences({}), device(VK_NULL_HANDLE), maxFrames(0)
{

}

VulkanSyncObjects::~VulkanSyncObjects()
{
	destroy();
}

void VulkanSyncObjects::create(VkDevice vkdevice, uint32_t maxFramesInFlight)
{
	device = vkdevice;
	maxFrames = maxFramesInFlight;

	imageAvailableSemaphores.resize(maxFramesInFlight);
	renderFinishedSemaphores.resize(maxFramesInFlight);
	inFlightFences.resize(maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void VulkanSyncObjects::destroy()
{
	if (!imageAvailableSemaphores.empty())
	{
		for (size_t i = 0; i < maxFrames; i++)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		}
		imageAvailableSemaphores.clear();
	}

	if (!renderFinishedSemaphores.empty())
	{
		for (size_t i = 0; i < maxFrames; i++)
		{
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		}
		renderFinishedSemaphores.clear();
	}

	if (!inFlightFences.empty())
	{
		for (size_t i = 0; i < maxFrames; i++)
		{
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		inFlightFences.clear();
	}
}

VkSemaphore VulkanSyncObjects::getImageAvailableSemaphore(uint32_t frameIndex) const
{
	if (imageAvailableSemaphores.empty())
	{
		throw std::runtime_error("Get imageAvailableSemaphore called before initalization!");
	}
	if (frameIndex < 0 || frameIndex >= maxFrames)
	{
		throw std::runtime_error("Invalid index for get imageAvailableSemaphore!");
	}
	return imageAvailableSemaphores[frameIndex];
}

VkSemaphore VulkanSyncObjects::getRenderFinishedSemaphore(uint32_t frameIndex) const
{
	if (renderFinishedSemaphores.empty())
	{
		throw std::runtime_error("Get renderFinishedSemaphore called before initalization!");
	}
	if (frameIndex < 0 || frameIndex >= maxFrames)
	{
		throw std::runtime_error("Invalid index for get renderFinishedSemaphore!");
	}
	return renderFinishedSemaphores[frameIndex];
}
VkFence VulkanSyncObjects::getInFlightFence(uint32_t frameIndex) const
{
	if (inFlightFences.empty())
	{
		throw std::runtime_error("Get inFlightFence called before initalization!");
	}
	if (frameIndex < 0 || frameIndex >= maxFrames)
	{
		throw std::runtime_error("Invalid index for get inFlightFence!");
	}
	return inFlightFences[frameIndex];
}