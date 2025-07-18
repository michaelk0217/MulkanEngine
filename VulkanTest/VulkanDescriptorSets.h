#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <map>

#include "VulkanTexture.h"
#include "VulkanUniformBuffers.h"
#include "Renderable.h"
#include "Lights.h"

struct IblPacket
{
	VkImageView irradianceImageView;
	VkSampler irradianceSampler;
	VkImageView prefilterImageView;
	VkSampler prefilterSampler;
	VkImageView brdfLutImageView;
	VkSampler brdfLutSampler;
};

class VulkanDescriptorSets
{
public:
	VulkanDescriptorSets();
	~VulkanDescriptorSets();

	void create(VkDevice vkdevice, VkDescriptorPool descriptorPool, 
				VkDescriptorSetLayout descriptorSetLayout, uint32_t numFrames, 
				const std::vector<VkBuffer>& frameUboBuffers,
				const std::vector<VkBuffer>& objectDUBuffers,
				VkImageView textureImageView, VkSampler textureSampler);

	static void createForMaterials(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t numFrames,
		const std::vector<VkBuffer> frameUboBuffers,
		const std::vector<VkBuffer> objectDUBuffers,
		const std::vector<VkBuffer> lightingUboBuffers,
		//const std::vector<VkBuffer> tessUboBuffers,
		const std::vector<VkBuffer> materialDataUboBuffers,
		std::map<std::string, std::shared_ptr<Material>>& materials,
		IblPacket iblPacket,
		size_t materialUboAlignedStride
	);

	void createForSkybox(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t numFrames,
		const std::vector<VkBuffer> frameUboBuffers,
		VulkanTexture& textureObj
	);

	void createForCubeMapConversion(
		VkDevice device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		VulkanTexture& textureObj
	);

	void destroy();

	VkDescriptorSet getVkDescriptorSet(uint32_t frameIndex) const;

	std::vector<VkDescriptorSet> getVkDescriptorSets() const;

	const VkDescriptorSet* getVkDescriptorSetsRaw() const;

private:
	std::vector<VkDescriptorSet> descriptorSets;

	VkDevice device;
};

