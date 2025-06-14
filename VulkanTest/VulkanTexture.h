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

	//void create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkCommandPool commandPool, VkQueue graphicsQueue, const std::string& path, bool skybox = false);
	void createTexture2D(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::string& path);
	void createTextureHDR(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::string& path);
	void createCubemap(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t width, uint32_t height);
	
	void destroy();

	VkImage getImage() const;
	VkImageView getImageView() const;
	VkSampler getSampler() const;

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkDevice device;

	void createTextureImageView(VkDevice vkdevice, VkFormat format);
	void createSkyboxHdrImageView(VkDevice vkdevice);
	void createTextureSampler(VkDevice vkdevice, VkPhysicalDevice vkphysdevice);

};

