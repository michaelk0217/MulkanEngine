#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <stdexcept>
class VulkanCommandBuffers
{
public:

	VulkanCommandBuffers();
	~VulkanCommandBuffers();

	void create(VkDevice vkdevice, VkCommandPool commandPool, uint32_t maxFrames);

	// records draw commands	
	static void recordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32_t currentFrame,
		VkRenderPass renderPass,
		VkPipelineLayout pipelineLayout,
		VkPipeline graphicsPipeline,
		VkFramebuffer swapChainFramebuffer,
		VkExtent2D swapChainExtent,
		const std::vector<VkDescriptorSet>& descriptorSets,
		VkBuffer vertexBuffer,
		VkBuffer indexBuffer,
		uint32_t indices_size);

	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	static void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool);

	VkCommandBuffer& getCommandBuffer(uint32_t index);

private:
	std::vector<VkCommandBuffer> commandBuffers;
	

};

