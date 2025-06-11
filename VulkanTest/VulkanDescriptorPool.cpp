#include "VulkanDescriptorPool.h"

VulkanDescriptorPool::VulkanDescriptorPool() : descriptorPool(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	destroy();
}

void VulkanDescriptorPool::create(VkDevice device, uint32_t maxFramesInFlight, uint32_t objectCount)
{
	this->device = device;

	uint32_t totalSetCount = maxFramesInFlight * objectCount;

	std::array<VkDescriptorPoolSize, 8> poolSizes{};
	// Frame Ubo
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = totalSetCount;
	// Object Ubo
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[1].descriptorCount = totalSetCount;
	// Lighting Ubo
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = totalSetCount;
	// albedo Sampler Ubo
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = totalSetCount;
	// normal Sampler Ubo
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[4].descriptorCount = totalSetCount;
	// orm Sampler Ubo
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[5].descriptorCount = totalSetCount;
	// displacement sampler Ubo
	poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[6].descriptorCount = totalSetCount;
	// tessellation Ubo
	poolSizes[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[7].descriptorCount = totalSetCount;


	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	//poolInfo.maxSets = static_cast<uint32_t>(maxFramesInFlight);
	poolInfo.maxSets = totalSetCount;

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
