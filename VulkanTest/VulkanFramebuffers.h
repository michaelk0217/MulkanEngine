#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <array>

class VulkanFramebuffers
{
public:
	VulkanFramebuffers();
	~VulkanFramebuffers();

	void create(VkDevice vkdevice, const std::vector<VkImageView>& swapChainImageViews, VkImageView depthImageView, VkRenderPass renderPass, VkExtent2D swapChainExtent);
	void createForImGui(VkDevice vkdevice, const std::vector<VkImageView>& swapChainImageViews, VkRenderPass renderPass, VkExtent2D swapChainExtent);
	void destroy();

	VkFramebuffer getFramebuffer(uint32_t index) const;

private:
	std::vector<VkFramebuffer> framebuffers;

	VkDevice device;
};

