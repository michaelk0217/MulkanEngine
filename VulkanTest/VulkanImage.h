#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "VulkanCommandBuffers.h"

//void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

//VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

//VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

class VulkanImage
{
public:
	static void createImage(
		VkDevice device, 
		VkPhysicalDevice physicalDevice, 
		uint32_t width, uint32_t height, 
		uint32_t mipLevels,
		uint32_t arrayLayers,
		VkFormat format, 
		VkImageTiling tiling, 
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, 
		VkImage& image, 
		VkDeviceMemory& imageMemory,
		VkImageCreateFlags flags = 0
	);

	static VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	static void createCubeMapImage(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		uint32_t width, 
		uint32_t height,
		uint32_t mipLevels,
		VkImage& image,
		VkDeviceMemory& imageMemory
	);

	static VkImageView createCubeMapView(VkDevice device, VkImage image, uint32_t mipLevel);

	static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	static void transitionImageLayout(
		VkDevice device, 
		VkQueue graphicsQueue, 
		VkCommandPool commandPool, 
		VkImage image, 
		VkFormat format, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout, 
		uint32_t layerCount = 1, 
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = 1
	);

	static void recordTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        uint32_t layerCount = 1,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 1
    );

	static void copyBufferToImage(VkDevice vkdevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

private:
	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	static bool hasStencilComponent(VkFormat format);
};

