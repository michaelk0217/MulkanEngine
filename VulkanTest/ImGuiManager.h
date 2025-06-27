#pragma once

#include <vulkan/vulkan.h>
#include <memory>

class Window;
class VulkanInstance;
class VulkanDevice;
class VulkanSurface;
class VulkanSwapChain;
class VulkanCommandPool;
//struct TessellationUBO;
struct SceneDebugContextPacket;

class ImGuiManager
{
public:
	ImGuiManager(
		Window& window,
		VulkanInstance& instance,
		VulkanDevice& device,
		VulkanSurface& surface,
		VulkanSwapChain& swapchain,
		VulkanCommandPool& commandPool
	);
	~ImGuiManager();

	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void newFrame();
	//void buildUI(bool& wireframeMode, TessellationUBO& tessUboData); // passing state
	void buildUI(SceneDebugContextPacket& sceneDebugContextPacket);
	//void render(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer);
	void render(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D swapChainExtent);

	static void check_vk_result(VkResult err);

	VkRenderPass getRenderPass() const;

private:
	void initRenderPass(VkFormat swapChainFormat);
	//VkExtent2D m_swapChainExtent;
	VulkanDevice& m_device;
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;

	void drawControlPanel(SceneDebugContextPacket& sceneDebugContextPacket);
	void drawLightingPanel(SceneDebugContextPacket& sceneDebugContextPacket);

};

