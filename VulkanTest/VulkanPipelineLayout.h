#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

class VulkanPipelineLayout
{
public:
	VulkanPipelineLayout();
	~VulkanPipelineLayout();

	void create(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);
	void destroy();

	VkPipelineLayout getVkPipelineLayout() const;

private:
	VkPipelineLayout pipelineLayout;

	VkDevice device;
};

