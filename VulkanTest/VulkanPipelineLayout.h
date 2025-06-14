#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

class VulkanPipelineLayout
{
public:
	VulkanPipelineLayout();
	~VulkanPipelineLayout();

	void create(
		VkDevice device, 
		VkDescriptorSetLayout descriptorSetLayout, 
		uint32_t pushConstantRangeCount = 0, 
		const VkPushConstantRange* pPushConstantRanges = nullptr
	);
	void destroy();

	VkPipelineLayout getVkPipelineLayout() const;

private:
	VkPipelineLayout pipelineLayout;

	VkDevice device;
};

