#include "VulkanPipelineLayout.h"

VulkanPipelineLayout::VulkanPipelineLayout() : device(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	destroy();
}

void VulkanPipelineLayout::create(
	VkDevice vkdevice, 
	VkDescriptorSetLayout descriptorSetLayout, 
	uint32_t pushConstantRangeCount, 
	const VkPushConstantRange* pPushConstantRanges)
{
	device = vkdevice;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;
	pipelineLayoutInfo.pPushConstantRanges = pPushConstantRanges;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void VulkanPipelineLayout::destroy()
{
	if (pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}
}

VkPipelineLayout VulkanPipelineLayout::getVkPipelineLayout() const
{
	if (pipelineLayout == VK_NULL_HANDLE)
	{
		throw std::runtime_error("PipelineLayout get called before creation!");
	}
	return pipelineLayout;
}
