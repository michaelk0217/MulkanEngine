#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <stdexcept>

class VulkanDescriptorSetLayout
{
public:
	VulkanDescriptorSetLayout();
	~VulkanDescriptorSetLayout();

	void create(VkDevice vkdevice);
	void destroy();

	VkDescriptorSetLayout getVkDescriptorSetLayout() const;

private:
	VkDescriptorSetLayout descriptorSetLayout;

	VkDevice device;
};

