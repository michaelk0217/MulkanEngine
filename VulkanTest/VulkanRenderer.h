// VulkanRenderer.h
#pragma once

#include <functional> // For std::function
#include <vector>
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanPipelineLayout.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFramebuffers.h"
#include "VulkanCommandBuffers.h"
#include "VulkanDescriptorSets.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanUniformBuffers.h"
#include "VulkanSyncObjects.h"
#include "ModelLoader.h" // For Vertex, UniformBufferObject (if not in a separate header)
#include "Renderable.h"

// Forward declare if UniformBufferObject is defined elsewhere and you don't want to include the full header
// struct UniformBufferObject;

class VulkanRenderer {
public:
    VulkanRenderer(
        VulkanDevice& device,
        VulkanSwapChain& swapChain,
        VulkanRenderPass& renderPass,
        VulkanFramebuffers& framebuffers,
        VulkanCommandBuffers& commandBuffers,
        VulkanSyncObjects& syncObjects,
        int maxFramesInFlight
    );

    ~VulkanRenderer();

    // Delete copy and move operations as this class will hold references
    // and manage a specific rendering context.
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&&) = delete;
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;

    uint32_t getCurrentFrame() const { return currentFrame; }

    // The main function to draw a frame
    // uboDataProvider: A function that returns the UBO for the current frame.
    // framebufferResized: A reference to the flag in the main application.
    // recreateSwapChainCallback: A function to call if the swapchain needs recreation from drawFrame.
    void drawFrame(
        RenderPacket& packet,
        bool& framebufferResized,
        std::function<void()> recreateSwapChainCallback
    );


    static void recordCommandBuffer(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrameIndex,
        VkRenderPass renderPass,
        VkPipelineLayout pipelineLayout,
        VkPipeline graphicsPipeline,
        VkFramebuffer swapChainFramebuffer,
        VkExtent2D swapChainExtent,
        const std::vector<RenderableObject>& renderables,
        VkDeviceSize dynamicUboAlignment,
        std::optional<SkyboxData> skyboxPacket
        /*VkPipeline skyboxGraphicsPipeline = VK_NULL_HANDLE,
        VkBuffer skyboxVertexBuffer = VK_NULL_HANDLE,
        std::vector<VkDescriptorSet> skyboxDescriptorSets = {},
        VkPipelineLayout skyboxPipelineLayout = VK_NULL_HANDLE*/
    );

private:
    // References to Vulkan components (owned by HelloTriangleApplication)
    VulkanDevice& devices; // Renamed to avoid conflict with member name in HelloTriangleApplication
    VulkanSwapChain& swapChainObj;
    VulkanRenderPass& vkRenderPass; // Using a different name to avoid confusion if original was also named renderPass
    VulkanFramebuffers& swapChainFramebuffersObj;
    VulkanCommandBuffers& vkCommandBuffers;
    VulkanSyncObjects& syncObjectsRef;
    const int MAX_FRAMES_IN_FLIGHT_RENDERER;

    uint32_t currentFrame = 0;
};