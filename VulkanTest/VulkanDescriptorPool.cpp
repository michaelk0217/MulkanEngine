#include "VulkanDescriptorPool.h"

VulkanDescriptorPool::VulkanDescriptorPool() : descriptorPool(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	destroy();
}

void VulkanDescriptorPool::create(VkDevice vkdevice, uint32_t maxFramesInFlight)
{
	device = vkdevice;

	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(maxFramesInFlight);


	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanDescriptorPool::destroy()
{
	if (descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}
}

VkDescriptorPool VulkanDescriptorPool::getVkDescriptorPool() const
{
	if (descriptorPool == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get descriptor pool called before initialization!");
	}
	return descriptorPool;
}
