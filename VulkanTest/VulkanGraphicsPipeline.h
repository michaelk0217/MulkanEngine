#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>

#include "ModelLoader.h"

class VulkanGraphicsPipeline
{
public:
	VulkanGraphicsPipeline();
	~VulkanGraphicsPipeline();
	
	void create(
		VkDevice vkDevice, 
		VkPipelineLayout pipelineLayout, 
		VkRenderPass renderPass, 
		const std::string& vertShaderPath, 
		const std::string& fragShaderPath, 
		const std::string& tescShaderPath, 
		const std::string& teseShaderPath,
		VkPolygonMode polygoneMode
	);

	void createSkybox(
		VkDevice vkDevice,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		const std::string& vertShaderPath,
		const std::string& fragShaderPath
	);

	void createForConversion(
		VkDevice device,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		const std::string& vertShaderPath,
		const std::string& fragShaderPath
	);

	void createForLutGeneration(
		VkDevice device,
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		const std::string& vertShaderPath,
		const std::string& fragShaderPath
	);

	void destroy();

	VkPipeline getVkPipeline() const;

private:
	VkPipeline graphicsPipeline;

	VkDevice device;

	static std::vector<char> readFile(const std::string& filename);

	static VkShaderModule createShaderModule(VkDevice vkdevice, const std::vector<char>& code);
};

