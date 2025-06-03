#include "VulkanDescriptorSets.h"

VulkanDescriptorSets::VulkanDescriptorSets() : descriptorSets({}), device(VK_NULL_HANDLE)
{
}

VulkanDescriptorSets::~VulkanDescriptorSets()
{
	destroy();
}

void VulkanDescriptorSets::create(VkDevice vkdevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, const std::vector<VkBuffer>& uniformBuffers, VkImageView textureImageView, VkSampler textureSampler)
{
	device = vkdevice;

	std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(numFrames);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < numFrames; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanDescriptorSets::destroy()
{
	// descriptor sets are destroyed when pool is destroyed
	descriptorSets.clear();
}

VkDescriptorSet VulkanDescriptorSets::getVkDescriptorSet(uint32_t frameIndex) const
{
	if (descriptorSets.empty())
	{
		throw std::runtime_error("Get descriptor set called before initalization!");
	}

	return descriptorSets[frameIndex];
}


std::vector<VkDescriptorSet> VulkanDescriptorSets::getVkDescriptorSets() const 
{
	if (descriptorSets.empty())
	{
		throw std::runtime_error("Get descriptor set called before initalization!");
	}

	return descriptorSets;
}
