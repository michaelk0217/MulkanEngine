#include "VulkanTexture.h"

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


VulkanTexture::VulkanTexture() : textureImage(VK_NULL_HANDLE), textureImageMemory(VK_NULL_HANDLE), textureImageView(VK_NULL_HANDLE), textureSampler(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
{
}

VulkanTexture::~VulkanTexture()
{
	destroy();
}

void VulkanTexture::destroy()
{
	if (device == VK_NULL_HANDLE)
	{
		return;
	}
	if (textureSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device, textureSampler, nullptr);
		textureSampler = VK_NULL_HANDLE;
	}

	if (textureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device, textureImageView, nullptr);
		textureImageView = VK_NULL_HANDLE;
	}
	
	if (textureImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(device, textureImage, nullptr);
		textureImage = VK_NULL_HANDLE;
	}

	if (textureImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, textureImageMemory, nullptr);
		textureImageMemory = VK_NULL_HANDLE;
	}
	device = VK_NULL_HANDLE;
}

VkImage VulkanTexture::getImage() const
{
	if (textureImage == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get texture image called before initialization!");
	}
	return textureImage;
}

VkImageView VulkanTexture::getImageView() const
{
	
	if (textureImageView == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get Texture image view called before initalization!");
	}
	return textureImageView;
}

VkSampler VulkanTexture::getSampler() const
{
	if (textureSampler == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get texture sampler called before initalization!");
	}
	return textureSampler;
}


void VulkanTexture::createTexture2D(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::string& path, bool sRGB)
{
	this->device = vkdevice;
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	VulkanBuffer::createBuffer(
		vkdevice,
		vkphysdevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(vkdevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vkdevice, stagingBufferMemory);

	stbi_image_free(pixels);

	VkFormat format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

	VulkanImage::createImage(
		vkdevice, vkphysdevice,
		texWidth, texHeight,
		1, 1,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory
	);

	VulkanImage::transitionImageLayout(vkdevice, graphicsQueue, commandPool, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VulkanImage::copyBufferToImage(vkdevice, commandPool, graphicsQueue, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	VulkanImage::transitionImageLayout(vkdevice, graphicsQueue, commandPool, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vkdevice, stagingBuffer, nullptr);
	vkFreeMemory(vkdevice, stagingBufferMemory, nullptr);

	createTextureImageView(device, VK_FORMAT_R8G8B8A8_SRGB);
	createTextureSampler(device, vkphysdevice, 1);
}

// This function loads the HDR but DOES NOT create a cubemap. It creates a simple 2D float texture.
void VulkanTexture::createTextureHDR(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, VkQueue graphicsQueue, VkCommandPool commandPool, const std::string& path)
{
	device = vkdevice;
	int texWidth, texHeight, texChannels;
	float* pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) { throw std::runtime_error("Failed to load HDR image file!"); }

	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);

	// Create staging buffer and copy data (this part of your code was correct)
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanBuffer::createBuffer(
		vkdevice,
		vkphysdevice,
		imageSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, stagingBufferMemory
	);
	void* data;
	vkMapMemory(vkdevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vkdevice, stagingBufferMemory);
	stbi_image_free(pixels);

	// Create the destination 2D image (NOT a cubemap)
	VulkanImage::createImage(
		vkdevice, vkphysdevice,
		texWidth, texHeight,
		1, 1,
		VK_FORMAT_R32G32B32A32_SFLOAT, // Float format for HDR
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // Just needs to be sampled
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory
	);

	// Transition layout and copy from staging buffer (this part of your code was also correct)
	VulkanImage::transitionImageLayout(vkdevice, graphicsQueue, commandPool, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VulkanImage::copyBufferToImage(vkdevice, commandPool, graphicsQueue, stagingBuffer, textureImage, texWidth, texHeight);
	VulkanImage::transitionImageLayout(vkdevice, graphicsQueue, commandPool, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vkdevice, stagingBuffer, nullptr);
	vkFreeMemory(vkdevice, stagingBufferMemory, nullptr);

	// Create the image view and sampler
	textureImageView = VulkanImage::createImageView(device, textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
	createTextureSampler(device, vkphysdevice, 1); // Assuming this is your existing sampler function
}

void VulkanTexture::createCubemap(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	device = vkdevice;
	VulkanImage::createCubeMapImage(vkdevice, vkphysdevice, width, height, mipLevels, textureImage, textureImageMemory);
	textureImageView = VulkanImage::createCubeMapView(vkdevice, textureImage, mipLevels);
	createTextureSampler(device, vkphysdevice, mipLevels);
}

void VulkanTexture::createRenderableTexture(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
{
	this->device = vkdevice;
	uint32_t mipLevels = 1;

	VulkanImage::createImage(
		device, vkphysdevice,
		width, height,
		mipLevels, 1,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory
	);

	textureImageView = VulkanImage::createImageView(
		device,
		textureImage,
		format,
		VK_IMAGE_ASPECT_COLOR_BIT
	);
	createTextureSampler(device, vkphysdevice, mipLevels);
}

void VulkanTexture::createTexture2DFromMemory(
	VkDevice device, 
	VkPhysicalDevice physicalDevice, 
	VkQueue graphicsQueue, 
	VkCommandPool commandPool, 
	const unsigned char* pixelData, 
	int width, int height, int channels, bool sRGB)
{
	this->device = device;

	if (!pixelData) {
		throw std::runtime_error("Cannot create texture from null pixel data!");
	}

	VkDeviceSize imageSize = width * height * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VulkanBuffer::createBuffer(
		device,
		physicalDevice,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

	if (channels == 4)
	{
		memcpy(data, pixelData, static_cast<size_t>(imageSize));
	}
	else if (channels == 3)
	{
		uint32_t* rgba_data = static_cast<uint32_t*>(data);
		for (size_t i = 0; i < (size_t)width * height; ++i)
		{
			rgba_data[i * 4 + 0] = pixelData[i * 3 + 0]; // R
			rgba_data[i * 4 + 1] = pixelData[i * 3 + 1]; // G
			rgba_data[i * 4 + 2] = pixelData[i * 3 + 2]; // B
			rgba_data[i * 4 + 3] = 255; // A (fully opaque)
		}
	}
	else
	{
		vkUnmapMemory(device, stagingBufferMemory);
		throw std::runtime_error("Unsupported texture channel count for in-memory loading: " + std::to_string(channels));
	}

	vkUnmapMemory(device, stagingBufferMemory);

	VkFormat format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
	VulkanImage::createImage(
		device, physicalDevice,
		width, height,
		1, 1,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory
	);

	VulkanImage::transitionImageLayout(
		device, graphicsQueue, commandPool,
		textureImage, format,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);
	VulkanImage::copyBufferToImage(
		device, commandPool, graphicsQueue,
		stagingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height)
	);

	VulkanImage::transitionImageLayout(
		device, graphicsQueue, commandPool,
		textureImage, format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	createTextureImageView(device, format);
	createTextureSampler(device, physicalDevice, 1);
}

void VulkanTexture::createTextureImageView(VkDevice vkdevice, VkFormat format)
{
	textureImageView = VulkanImage::createImageView(vkdevice, textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::createSkyboxHdrImageView(VkDevice vkdevice)
{
	textureImageView = VulkanImage::createCubeMapView(vkdevice, textureImage, 1);
}



void VulkanTexture::createTextureSampler(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t mipLevels)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vkphysdevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	if (vkCreateSampler(vkdevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create textures sampler!");
	}
}