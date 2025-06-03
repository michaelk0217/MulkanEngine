#include "VulkanFramebuffers.h"

VulkanFramebuffers::VulkanFramebuffers() : framebuffers({}), device(VK_NULL_HANDLE)
{
}

VulkanFramebuffers::~VulkanFramebuffers()
{
	destroy();
}

void VulkanFramebuffers::create(VkDevice vkdevice, const std::vector<VkImageView>& swapChainImageViews, VkImageView depthImageView, VkRenderPass renderPass, VkExtent2D swapChainExtent)
{
	device = vkdevice;
	framebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanFramebuffers::destroy()
{
	for (auto framebuffer : framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	framebuffers.clear();
}

VkFramebuffer VulkanFramebuffers::getFramebuffer(uint32_t index) const
{
	if (framebuffers.size() == 0)
	{
		throw std::runtime_error("Get framebuffers called before initalization!");
	}

	if (index >= framebuffers.size() || index < 0)
	{
		throw std::runtime_error("Invalid framebuffer index!");
	}

	return framebuffers[index];
}
