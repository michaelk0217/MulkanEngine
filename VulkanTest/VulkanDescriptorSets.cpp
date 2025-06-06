#include "VulkanDescriptorSets.h"

VulkanDescriptorSets::VulkanDescriptorSets() : descriptorSets({}), device(VK_NULL_HANDLE)
{
}

VulkanDescriptorSets::~VulkanDescriptorSets()
{
	destroy();
}

void VulkanDescriptorSets::create(VkDevice vkdevice, VkDescriptorPool descriptorPool, 
							VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, 
							const std::vector<VkBuffer>& frameUboBuffers,
							const std::vector<VkBuffer>& objectDUBuffers,
							VkImageView textureImageView, VkSampler textureSampler)
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

	for (size_t i = 0; i < numFrames; i++) 
	{
		VkDescriptorBufferInfo frameBufferInfo{};
		frameBufferInfo.buffer = frameUboBuffers[i];
		frameBufferInfo.offset = 0;
		frameBufferInfo.range = sizeof(FrameUniformBufferObject);

		VkDescriptorBufferInfo objectBufferInfo{};
		objectBufferInfo.buffer = objectDUBuffers[i];
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(ObjectUniformBufferObject); // size of ONE object's data for DYNAMIC UBO

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &frameBufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &objectBufferInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}



// VulkanDescriptorSets.cpp
void VulkanDescriptorSets::createForRenderables(
	VkDevice device,
	VkDescriptorPool descriptorPool,
	VkDescriptorSetLayout descriptorSetLayout,
	uint32_t numFrames,
	const std::vector<VkBuffer> frameUboBuffers,
	const std::vector<VkBuffer> objectDUBuffers,
	std::vector<RenderableObject>& renderables) // Note the &
{
	this->device = device;

	// Loop through each object that needs descriptor sets
	for (auto& renderable : renderables)
	{
		std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
		allocInfo.pSetLayouts = layouts.data();

		// Allocate the sets directly into the renderable object's vector
		// The vector should already be sized correctly by the RenderableObject constructor.
		if (vkAllocateDescriptorSets(device, &allocInfo, renderable.frameSpecificDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets for a renderable object!");
		}

		// Now, update these newly allocated sets for each frame
		for (size_t i = 0; i < numFrames; i++)
		{
			VkDescriptorBufferInfo frameBufferInfo{};
			frameBufferInfo.buffer = frameUboBuffers[i];
			frameBufferInfo.offset = 0;
			frameBufferInfo.range = sizeof(FrameUniformBufferObject);

			VkDescriptorBufferInfo objectBufferInfo{};
			objectBufferInfo.buffer = objectDUBuffers[i];
			objectBufferInfo.offset = 0;
			// This is key for dynamic UBOs
			objectBufferInfo.range = sizeof(ObjectUniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = renderable.texture->getImageView();
			imageInfo.sampler = renderable.texture->getSampler();

			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

			// Frame UBO
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = renderable.frameSpecificDescriptorSets[i]; // Use the allocated set
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &frameBufferInfo;

			// Object Dynamic UBO
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = renderable.frameSpecificDescriptorSets[i]; // Use the allocated set
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &objectBufferInfo;

			// Texture Sampler
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = renderable.frameSpecificDescriptorSets[i]; // Use the allocated set
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

//void VulkanDescriptorSets::createForRenderables(
//	VkDevice device, 
//	VkDescriptorPool descriptorPool, 
//	VkDescriptorSetLayout descriptorSetLayout, 
//	uint32_t numFrames, 
//	const std::vector<VkBuffer> frameUboBuffers, 
//	const std::vector<VkBuffer> objectDUBuffers, 
//	std::vector<RenderableObject>& renderables)
//{
//	this->device = device;
//
//	std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
//	VkDescriptorSetAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	allocInfo.descriptorPool = descriptorPool;
//	allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
//	allocInfo.pSetLayouts = layouts.data();
//
//	descriptorSets.resize(numFrames);
//	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to allocate descriptor sets!");
//	}
//
//	for (auto& renderable : renderables)
//	{
//		std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
//		VkDescriptorSetAllocateInfo allocInfo{};
//		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//		allocInfo.descriptorPool = descriptorPool;
//		allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
//		allocInfo.pSetLayouts = layouts.data();
//
//		descriptorSets.resize(numFrames);
//		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
//		{
//			throw std::runtime_error("failed to allocate descriptor sets!");
//		}
//
//		for (size_t i = 0; i < numFrames; i++)
//		{
//			VkDescriptorBufferInfo frameBufferInfo{};
//			frameBufferInfo.buffer = frameUboBuffers[i];
//			frameBufferInfo.offset = 0;
//			frameBufferInfo.range = sizeof(FrameUniformBufferObject);
//
//			VkDescriptorBufferInfo objectBufferInfo{};
//			objectBufferInfo.buffer = objectDUBuffers[i];
//			objectBufferInfo.offset = 0;
//			objectBufferInfo.range = sizeof(ObjectUniformBufferObject); // size of ONE object's data for DYNAMIC UBO
//
//			VkDescriptorImageInfo imageInfo{};
//			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//			imageInfo.imageView = renderable.texture->getImageView();
//			imageInfo.sampler = renderable.texture->getSampler();
//
//			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
//
//			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			descriptorWrites[0].dstSet = descriptorSets[i];
//			descriptorWrites[0].dstBinding = 0;
//			descriptorWrites[0].dstArrayElement = 0;
//			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//			descriptorWrites[0].descriptorCount = 1;
//			descriptorWrites[0].pBufferInfo = &frameBufferInfo;
//
//			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			descriptorWrites[1].dstSet = descriptorSets[i];
//			descriptorWrites[1].dstBinding = 1;
//			descriptorWrites[1].dstArrayElement = 0;
//			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
//			descriptorWrites[1].descriptorCount = 1;
//			descriptorWrites[1].pBufferInfo = &objectBufferInfo;
//
//			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			descriptorWrites[2].dstSet = descriptorSets[i];
//			descriptorWrites[2].dstBinding = 2;
//			descriptorWrites[2].dstArrayElement = 0;
//			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//			descriptorWrites[2].descriptorCount = 1;
//			descriptorWrites[2].pImageInfo = &imageInfo;
//
//			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
//		}
//	}
//}



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
