#include "VulkanUniformBuffers.h"

VulkanUniformBuffers::VulkanUniformBuffers() : uniformBuffers({}), uniformBuffersMemory({}), uniformBuffersMapped({}), device(VK_NULL_HANDLE), frameCount(0)
{
}

VulkanUniformBuffers::~VulkanUniformBuffers()
{
	destroy();
}

void VulkanUniformBuffers::create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t numFrames)
{
	device = vkdevice;
	frameCount = numFrames;

	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(frameCount);
	uniformBuffersMemory.resize(frameCount);
	uniformBuffersMapped.resize(frameCount);

	for (size_t i = 0; i < frameCount; i++)
	{

		VulkanBuffer::createBuffer(
			device,
			vkphysdevice,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]
		);

		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}

void VulkanUniformBuffers::destroy()
{
	if (!uniformBuffers.empty())
	{
		for (size_t i = 0; i < frameCount; i++)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		}
		uniformBuffers.clear();
	}

	if (!uniformBuffersMemory.empty())
	{
		for (size_t i = 0; i < frameCount; i++)
		{
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		uniformBuffersMemory.clear();
	}
}

void VulkanUniformBuffers::update(uint32_t frameIndex, const UniformBufferObject ubo)
{
	memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

VkBuffer VulkanUniformBuffers::getBuffer(uint32_t frameIndex) const
{
	if (uniformBuffers.size() == 0)
	{
		throw std::runtime_error("Uniform buffer get called before initalization!");
	}
	if (frameIndex < 0 || frameIndex >= frameCount)
	{
		throw std::runtime_error("Get uniform buffer: invalid index");
	}


	return uniformBuffers[frameIndex];
}

std::vector<VkBuffer> VulkanUniformBuffers::getBuffers() const
{
	if (uniformBuffers.empty())
	{
		throw std::runtime_error("Uniform buffer get called before initalization!");
	}
	return uniformBuffers;
}
