#include "VulkanDescriptorSets.h"
#include "iostream"

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

void VulkanDescriptorSets::createForMaterials(
	VkDevice device, VkDescriptorPool descriptorPool, 
	VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, 
	const std::vector<VkBuffer> frameUboBuffers, const std::vector<VkBuffer> objectDUBuffers, 
	const std::vector<VkBuffer> lightingUboBuffers, const std::vector<VkBuffer> tessUboBuffers,
	std::map<std::string, std::shared_ptr<Material>>& materials,
	IblPacket iblPacket)
{
	this->device = device;

	for (auto& pair : materials)
	{
		std::shared_ptr<Material> material = pair.second;
		
		material->frameSpecificDescriptorSets.resize(numFrames);

		std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
		allocInfo.pSetLayouts = layouts.data();

		std::cout << "Allocating Descriptor Sets for " << material->name << std::endl;
		if (vkAllocateDescriptorSets(device, &allocInfo, material->frameSpecificDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets for a material: " + material->name);
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
			// This is key for dynamic UBOs
			objectBufferInfo.range = sizeof(ObjectUniformBufferObject);

			VkDescriptorBufferInfo lightingBufferInfo{};
			lightingBufferInfo.buffer = lightingUboBuffers[i];
			lightingBufferInfo.offset = 0;
			lightingBufferInfo.range = sizeof(SceneLightingUBO);

			VkDescriptorImageInfo albedoMapImageInfo{};
			albedoMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			albedoMapImageInfo.imageView = material->albedoMap->getImageView();
			albedoMapImageInfo.sampler = material->albedoMap->getSampler();

			VkDescriptorImageInfo normalMapImageInfo{};
			normalMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			normalMapImageInfo.imageView = material->normalMap->getImageView();
			normalMapImageInfo.sampler = material->normalMap->getSampler();

			VkDescriptorImageInfo ormMapImageInfo{};
			ormMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ormMapImageInfo.imageView = material->ormMap->getImageView();
			ormMapImageInfo.sampler = material->ormMap->getSampler();

			VkDescriptorImageInfo aoMapImageInfo{};
			aoMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			aoMapImageInfo.imageView = material->aoMap->getImageView();
			aoMapImageInfo.sampler = material->aoMap->getSampler();

			VkDescriptorImageInfo roughnessMapImageInfo{};
			roughnessMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			roughnessMapImageInfo.imageView = material->roughnessMap->getImageView();
			roughnessMapImageInfo.sampler = material->roughnessMap->getSampler();

			VkDescriptorImageInfo metallnessMapImageInfo{};
			metallnessMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			metallnessMapImageInfo.imageView = material->metallnessMap->getImageView();
			metallnessMapImageInfo.sampler = material->metallnessMap->getSampler();

			VkDescriptorImageInfo displacementMapImageInfo{};
			displacementMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			displacementMapImageInfo.imageView = material->displacementMap->getImageView();
			displacementMapImageInfo.sampler = material->displacementMap->getSampler();

			VkDescriptorBufferInfo tessBufferInfo{};
			tessBufferInfo.buffer = tessUboBuffers[i];
			tessBufferInfo.offset = 0;
			tessBufferInfo.range = sizeof(TessellationUBO);

			VkDescriptorImageInfo irradianceMapImageInfo{};
			irradianceMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			irradianceMapImageInfo.imageView = iblPacket.irradianceImageView;
			irradianceMapImageInfo.sampler = iblPacket.irradianceSampler;

			VkDescriptorImageInfo prefilterMapImageInfo{};
			prefilterMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			prefilterMapImageInfo.imageView = iblPacket.prefilterImageView;
			prefilterMapImageInfo.sampler = iblPacket.prefilterSampler;

			VkDescriptorImageInfo brdfLutMapImageInfo{};
			brdfLutMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			brdfLutMapImageInfo.imageView = iblPacket.brdfLutImageView;
			brdfLutMapImageInfo.sampler = iblPacket.brdfLutSampler;

			std::array<VkWriteDescriptorSet, 14> descriptorWrites{};

			// Frame UBO
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &frameBufferInfo;

			// Object UBO
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &objectBufferInfo;

			// Lighting UBO
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &lightingBufferInfo;

			// Material Albedo Map UBO
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &albedoMapImageInfo;

			// Material Normal Map UBO
			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &normalMapImageInfo;

			// Material ORM Map UBO
			descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[5].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[5].dstBinding = 5;
			descriptorWrites[5].dstArrayElement = 0;
			descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[5].descriptorCount = 1;
			descriptorWrites[5].pImageInfo = &ormMapImageInfo;

			// Material AO Map UBO
			descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[6].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[6].dstBinding = 6;
			descriptorWrites[6].dstArrayElement = 0;
			descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[6].descriptorCount = 1;
			descriptorWrites[6].pImageInfo = &aoMapImageInfo;
			// Material Roughness Map UBO
			descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[7].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[7].dstBinding = 7;
			descriptorWrites[7].dstArrayElement = 0;
			descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[7].descriptorCount = 1;
			descriptorWrites[7].pImageInfo = &roughnessMapImageInfo;
			// Material Metallness Map UBO
			descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[8].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[8].dstBinding = 8;
			descriptorWrites[8].dstArrayElement = 0;
			descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[8].descriptorCount = 1;
			descriptorWrites[8].pImageInfo = &metallnessMapImageInfo;

			// Material Displacement Map UBO
			descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[9].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[9].dstBinding = 9;
			descriptorWrites[9].dstArrayElement = 0;
			descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[9].descriptorCount = 1;
			descriptorWrites[9].pImageInfo = &displacementMapImageInfo;

			// Tessellation UBO
			descriptorWrites[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[10].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[10].dstBinding = 10;
			descriptorWrites[10].dstArrayElement = 0;
			descriptorWrites[10].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[10].descriptorCount = 1;
			descriptorWrites[10].pBufferInfo = &tessBufferInfo;

			// Irradiance Map UBO
			descriptorWrites[11].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[11].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[11].dstBinding = 11;
			descriptorWrites[11].dstArrayElement = 0;
			descriptorWrites[11].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[11].descriptorCount = 1;
			descriptorWrites[11].pImageInfo = &irradianceMapImageInfo;

			// Prefilter Map UBO
			descriptorWrites[12].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[12].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[12].dstBinding = 12;
			descriptorWrites[12].dstArrayElement = 0;
			descriptorWrites[12].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[12].descriptorCount = 1;
			descriptorWrites[12].pImageInfo = &prefilterMapImageInfo;

			// BrdfLut Map UBO
			descriptorWrites[13].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[13].dstSet = material->frameSpecificDescriptorSets[i];
			descriptorWrites[13].dstBinding = 13;
			descriptorWrites[13].dstArrayElement = 0;
			descriptorWrites[13].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[13].descriptorCount = 1;
			descriptorWrites[13].pImageInfo = &brdfLutMapImageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

void VulkanDescriptorSets::createForSkybox(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, const std::vector<VkBuffer> frameUboBuffers, VulkanTexture& textureObj)
{
	std::vector<VkDescriptorSetLayout> layouts(numFrames, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numFrames);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(numFrames);

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets for skybox.");
	}

	for (size_t i = 0; i < numFrames; i++)
	{
		VkDescriptorBufferInfo frameBufferInfo{};
		frameBufferInfo.buffer = frameUboBuffers[i];
		frameBufferInfo.offset = 0;
		frameBufferInfo.range = sizeof(FrameUniformBufferObject);

		VkDescriptorImageInfo skyboxImageInfo{};
		skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		skyboxImageInfo.imageView = textureObj.getImageView();
		skyboxImageInfo.sampler = textureObj.getSampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// Frame UBO
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &frameBufferInfo;

		// Skybox Hdr Sampling
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &skyboxImageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanDescriptorSets::createForCubeMapConversion(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VulkanTexture& textureObj)
{
	this->device = device;
	std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(1);

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets for cubemap conversion.");
	}

	VkDescriptorImageInfo cubemapImageInfo{};
	cubemapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	cubemapImageInfo.imageView = textureObj.getImageView();
	cubemapImageInfo.sampler = textureObj.getSampler();

	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSets[0];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pImageInfo = &cubemapImageInfo;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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

const VkDescriptorSet* VulkanDescriptorSets::getVkDescriptorSetsRaw() const
{
	if (descriptorSets.empty())
	{
		throw std::runtime_error("Get descriptor set called before initialization!");
	}

	return descriptorSets.data();
}
