#include "VulkanDescriptorPool.h"

VulkanDescriptorPool::VulkanDescriptorPool() : descriptorPool(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
{
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	destroy(); 
}

void VulkanDescriptorPool::create(VkDevice vkdevice, uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes)
{
	this->device = vkdevice;

	//// We only need 3 pool size entries, one for each unique descriptor type we use.
	//std::array<VkDescriptorPoolSize, 3> poolSizes{};

	//// --- UNIFORM BUFFERS (Standard) ---
	//// PBR Materials: (Frame + Lighting + MaterialData) * objectCount * frames
	//// Skybox: (Frame) * frames
	//// One-off IBL sets: 2 (irradiance and prefilter)
	//poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//poolSizes[0].descriptorCount = (objectCount * 3 + 1) * maxFramesInFlight + 2;

	//// --- UNIFORM BUFFERS (Dynamic) ---
	//// PBR Materials: (Object) * objectCount * frames
	//poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	//poolSizes[1].descriptorCount = objectCount * maxFramesInFlight;

	//// --- COMBINED IMAGE SAMPLERS ---
	//poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//// PBR Materials: 5 base textures  + 3 IBL textures
	//uint32_t pbrMaterialSamplers = objectCount * 8 * maxFramesInFlight;
	//uint32_t skyboxSamplers = 1 * maxFramesInFlight;
	//// 3 one-time conversion passes, each use one sampler description set.
	//// HDR->Skybox, Skybox->Irradiance, Skybox->Prefilter
	//uint32_t conversionSamplers = 3;

	//poolSizes[2].descriptorCount = pbrMaterialSamplers + skyboxSamplers + conversionSamplers;

	//uint32_t pbrSets = maxFramesInFlight * objectCount;
	//uint32_t skyboxSets = maxFramesInFlight;
	//uint32_t conversionSets = 3;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Good practice to add this flag
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = maxSets;

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
