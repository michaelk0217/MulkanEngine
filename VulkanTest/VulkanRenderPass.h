#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <array>

#include "VulkanDepthResources.h"

class VulkanRenderPass
{
public:
	VulkanRenderPass();
	~VulkanRenderPass();

	void create(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat colorAttachmentFormat);
	void offscreen_rendering_create(VkDevice device, VkPhysicalDevice physicalDevice);
	void destroy();


	VkRenderPass getVkRenderPass() const;
private:
	VkRenderPass renderPass;

	VkDevice device;
};

