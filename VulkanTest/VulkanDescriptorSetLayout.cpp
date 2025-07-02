#include "VulkanDescriptorSetLayout.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout() : device(VK_NULL_HANDLE), descriptorSetLayout(VK_NULL_HANDLE)
{
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	destroy();
}

void VulkanDescriptorSetLayout::create(VkDevice vkdevice)
{
	device = vkdevice;

	VkDescriptorSetLayoutBinding frameUboLayoutBinding{};
	frameUboLayoutBinding.binding = 0; // For FrameUBO (view/proj)
	frameUboLayoutBinding.descriptorCount = 1;
	frameUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frameUboLayoutBinding.pImmutableSamplers = nullptr;
	frameUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT/* | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT*/;

	VkDescriptorSetLayoutBinding objectUboLayoutBinding{};
	objectUboLayoutBinding.binding = 1;
	objectUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	objectUboLayoutBinding.descriptorCount = 1;
	objectUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT/* | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT*/;
	objectUboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding lightingUboLayoutBinding{};
	lightingUboLayoutBinding.binding = 2;
	lightingUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightingUboLayoutBinding.descriptorCount = 1;
	lightingUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	lightingUboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding materialUboLayoutBinding{};
	materialUboLayoutBinding.binding = 3;
	materialUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	materialUboLayoutBinding.descriptorCount = 1;
	materialUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialUboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding albedoSamplerLayoutBinding{};
	albedoSamplerLayoutBinding.binding = 4; 
	albedoSamplerLayoutBinding.descriptorCount = 1;
	albedoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoSamplerLayoutBinding.pImmutableSamplers = nullptr;
	albedoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
	normalSamplerLayoutBinding.binding = 5;
	normalSamplerLayoutBinding.descriptorCount = 1;
	normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
	normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding metallicRoughnessSamplerLayoutBinding{};
	metallicRoughnessSamplerLayoutBinding.binding = 6;
	metallicRoughnessSamplerLayoutBinding.descriptorCount = 1;
	metallicRoughnessSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	metallicRoughnessSamplerLayoutBinding.pImmutableSamplers = nullptr;
	metallicRoughnessSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding occlusionMapSamplerLayoutBinding{};
	occlusionMapSamplerLayoutBinding.binding = 7;
	occlusionMapSamplerLayoutBinding.descriptorCount = 1;
	occlusionMapSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	occlusionMapSamplerLayoutBinding.pImmutableSamplers = nullptr;
	occlusionMapSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding emissiveSamplerLayoutBinding{};
	emissiveSamplerLayoutBinding.binding = 8;
	emissiveSamplerLayoutBinding.descriptorCount = 1;
	emissiveSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	emissiveSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	emissiveSamplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding irradianceSamplerLayoutBinding{};
	irradianceSamplerLayoutBinding.binding = 9;
	irradianceSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	irradianceSamplerLayoutBinding.descriptorCount = 1;
	irradianceSamplerLayoutBinding.pImmutableSamplers = nullptr;
	irradianceSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding prefilterSamplerLayoutBinding{};
	prefilterSamplerLayoutBinding.binding = 10;
	prefilterSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	prefilterSamplerLayoutBinding.descriptorCount = 1;
	prefilterSamplerLayoutBinding.pImmutableSamplers = nullptr;
	prefilterSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding brdfLutSamplerLayoutBinding{};
	brdfLutSamplerLayoutBinding.binding = 11;
	brdfLutSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	brdfLutSamplerLayoutBinding.descriptorCount = 1;
	brdfLutSamplerLayoutBinding.pImmutableSamplers = nullptr;
	brdfLutSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 12> bindings = { 
		frameUboLayoutBinding, 
		objectUboLayoutBinding,
		lightingUboLayoutBinding,
		materialUboLayoutBinding,
		albedoSamplerLayoutBinding,
		normalSamplerLayoutBinding,
		metallicRoughnessSamplerLayoutBinding,
		occlusionMapSamplerLayoutBinding,
		emissiveSamplerLayoutBinding,
		irradianceSamplerLayoutBinding,
		prefilterSamplerLayoutBinding,
		brdfLutSamplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VulkanDescriptorSetLayout::createForSkybox(VkDevice device)
{
	this->device = device;

	VkDescriptorSetLayoutBinding frameUboLayoutBinding{};
	frameUboLayoutBinding.binding = 0; // For FrameUBO (view/proj)
	frameUboLayoutBinding.descriptorCount = 1;
	frameUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frameUboLayoutBinding.pImmutableSamplers = nullptr;
	frameUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding skyboxSamplerBinding{};
	skyboxSamplerBinding.binding = 1;
	skyboxSamplerBinding.descriptorCount = 1;
	skyboxSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	skyboxSamplerBinding.pImmutableSamplers = nullptr;
	skyboxSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
		frameUboLayoutBinding,
		skyboxSamplerBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create skybox descriptor set layout!");
	}
}

void VulkanDescriptorSetLayout::createForCubmapConversion(VkDevice device)
{
	this->device = device;

	VkDescriptorSetLayoutBinding hdrSamplerBinding{};
	hdrSamplerBinding.binding = 0;
	hdrSamplerBinding.descriptorCount = 1;
	hdrSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	hdrSamplerBinding.pImmutableSamplers = nullptr;
	hdrSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
		hdrSamplerBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create cubmap conversion descriptor set layout!");
	}
}


void VulkanDescriptorSetLayout::destroy()
{
	if (descriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		descriptorSetLayout = VK_NULL_HANDLE;
	}
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::getVkDescriptorSetLayout() const
{
	if (descriptorSetLayout == VK_NULL_HANDLE)
	{
		throw std::runtime_error("get descriptorSetLayout called before creation!");
	}
	return descriptorSetLayout;
}
