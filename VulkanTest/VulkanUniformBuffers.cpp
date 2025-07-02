#include "VulkanUniformBuffers.h"

VulkanUniformBuffers::VulkanUniformBuffers()
	: uniformBuffers({}), uniformBuffersMemory({}), uniformBuffersMapped({}),
	device(VK_NULL_HANDLE), frameCount(0), isDynamic(false), dynamicAlignment(0), bufferSize(0)
{
}

VulkanUniformBuffers::~VulkanUniformBuffers()
{
	destroy();
}

void VulkanUniformBuffers::create(VkDevice vkdevice, VkPhysicalDevice vkphysdevice, uint32_t numFrames,
									VkDeviceSize totalBufferSize,
										bool isDynamic)
{
	device = vkdevice;
	frameCount = numFrames;
	this->isDynamic = isDynamic;
	this->bufferSize = totalBufferSize;

	//VkDeviceSize bufferSize = sizeof(FrameUniformBufferObject);

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(vkphysdevice, &properties);
	dynamicAlignment = properties.limits.minUniformBufferOffsetAlignment;

	uniformBuffers.resize(frameCount);
	uniformBuffersMemory.resize(frameCount);
	uniformBuffersMapped.resize(frameCount);

	if (isDynamic)
	{
		for (size_t i = 0; i < frameCount; i++)
		{
			VulkanBuffer::createBuffer(
				device,
				vkphysdevice,
				totalBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i],
				uniformBuffersMemory[i]
			);

			vkMapMemory(device, uniformBuffersMemory[i], 0, totalBufferSize, 0, &uniformBuffersMapped[i]);
		}
	}
	else
	{
		VkDeviceSize bufferSize = sizeof(FrameUniformBufferObject);
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

	
}

void VulkanUniformBuffers::destroy()
{
	if (!uniformBuffers.empty())
	{
		for (size_t i = 0; i < frameCount; i++)
		{
			if (uniformBuffers[i] != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(device, uniformBuffers[i], nullptr);
				uniformBuffers[i] = VK_NULL_HANDLE;
			}
		}
		uniformBuffers.clear();
	}

	if (!uniformBuffersMemory.empty())
	{
		for (size_t i = 0; i < frameCount; i++)
		{
			if (uniformBuffersMemory[i] != VK_NULL_HANDLE)
			{
				vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
				uniformBuffersMemory[i] = VK_NULL_HANDLE;
			}
		}
		uniformBuffersMemory.clear();
	}
	uniformBuffersMapped.clear();
}
template<typename T>
void VulkanUniformBuffers::update(uint32_t frameIndex, const T& ubo)
{
	if (frameIndex >= frameCount)
	{
		throw std::runtime_error("Update uniform buffer: invalid frame index");
	}

	if (isDynamic)
	{
		throw std::runtime_error("Update function not implemented for dynamic UBOs");
	}
	
	if (sizeof(T) != bufferSize)
	{
		throw std:: runtime_error("UBO size mismatch: provided UBO size does not match buffer size.");
	}

	memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(T));
}

void VulkanUniformBuffers::updateDynamic(uint32_t frameIndex, uint32_t objectIndex, const ObjectUniformBufferObject& ubo)
{
	if (!isDynamic)
	{
		throw std::runtime_error("updateDynamic called on non-dynamic UBO");
	}
	if (frameIndex >= frameCount)
	{
		throw std::runtime_error("Update dynamic uniform buffer: invalid frame index");
	}

	// Calculate the aligned offset for the object
	VkDeviceSize offset = objectIndex * dynamicAlignment;
	memcpy(static_cast<char*>(uniformBuffersMapped[frameIndex]) + offset, &ubo, sizeof(ubo));
}

// ONLY CALL IF UBO IS FOR UPDATING SceneLightingUBO!!!
void VulkanUniformBuffers::updateLights(uint32_t frameIndex, SceneLightingUBO& lightUbo)
{
	if (frameIndex >= frameCount)
	{
		throw std::runtime_error("Update lights uniform buffer: invalid frame index");
	}

	memcpy(uniformBuffersMapped[frameIndex], &lightUbo, sizeof(lightUbo));
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

void* VulkanUniformBuffers::getMappedMemory(uint32_t frameIndex) const
{
	return uniformBuffersMapped[frameIndex];
}

VkDeviceSize VulkanUniformBuffers::totalObjectDataBufferSize(VkPhysicalDevice physdevice)
{
	VkDeviceSize objectDataUboAlignedSize;
	
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physdevice, &properties);
	VkDeviceSize minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;

	objectDataUboAlignedSize = (sizeof(ObjectUniformBufferObject) + minUboAlignment - 1) & ~(minUboAlignment - 1);

	return objectDataUboAlignedSize * VulkanGlobals::MAX_EXPECTED_OBJECTS;
}

// Explicit template instantiations
template void VulkanUniformBuffers::update<FrameUniformBufferObject>(uint32_t frameIndex, const FrameUniformBufferObject& ubo);
template void VulkanUniformBuffers::update<TessellationUBO>(uint32_t frameIndex, const TessellationUBO& ubo);
template void VulkanUniformBuffers::update<SceneLightingUBO>(uint32_t frameIndex, const SceneLightingUBO& ubo);