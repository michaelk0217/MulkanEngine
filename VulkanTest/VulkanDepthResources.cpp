#include "VulkanDepthResources.h"

VulkanDepthResources::VulkanDepthResources() : device(VK_NULL_HANDLE), depthImage(VK_NULL_HANDLE), depthImageMemory(VK_NULL_HANDLE), depthImageView(VK_NULL_HANDLE)
{
}

VulkanDepthResources::~VulkanDepthResources()
{
	destroy();
}

void VulkanDepthResources::create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, VkExtent2D swapChainExtent)
{
	device = vkdevice;
	VkFormat depthFormat = findDepthFormat(vkphysdevice);
	VulkanImage::createImage(device, vkphysdevice, swapChainExtent.width, swapChainExtent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = VulkanImage::createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	VulkanImage::transitionImageLayout(device, graphicsQueue, commandPool, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanDepthResources::destroy()
{
	if (depthImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		depthImageView = VK_NULL_HANDLE;
	}

	if (depthImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(device, depthImage, nullptr);
		depthImage = VK_NULL_HANDLE;
	}

	if (depthImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, depthImageMemory, nullptr);
		depthImageMemory = VK_NULL_HANDLE;
	}
}

VkImageView VulkanDepthResources::getDepthImageView() const
{
	if (depthImageView == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get depth image view called before initalization!");
	}
	return depthImageView;
}

VkFormat VulkanDepthResources::findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return VulkanImage::findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}