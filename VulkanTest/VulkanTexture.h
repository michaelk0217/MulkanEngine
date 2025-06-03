#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>

#include "VulkanImage.h"
#include "VulkanBuffer.h"

class VulkanTexture
{
public:
	VulkanTexture();
	~VulkanTexture();

	void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& path);
	void destroy();

	VkImageView getImageView() const;
	VkSampler getSampler() const;

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkDevice device;

	void createTextureImage(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::string& path);
	void createTextureImageView(VkDevice vkdevice);
	void createTextureSampler(VkDevice vkdevice, VkPhysicalDevice vkphysdevice);

};

