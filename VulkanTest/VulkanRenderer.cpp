#include "VulkanRenderer.h"
#include <stdexcept> // For runtime_error
#include <iostream>  // For debug/error output

VulkanRenderer::VulkanRenderer(
    VulkanDevice& device,
    VulkanSwapChain& swapChain,
    VulkanRenderPass& renderPass,
    VulkanPipelineLayout& pipelineLayout,
    VulkanGraphicsPipeline& graphicsPipeline,
    VulkanFramebuffers& framebuffers,
    VulkanCommandBuffers& commandBuffers,
    VulkanUniformBuffers& uniformBuffers,
    VulkanSyncObjects& syncObjects,
    int maxFramesInFlight
) : devices(device),
swapChainObj(swapChain),
vkRenderPass(renderPass),
vkPipelineLayout(pipelineLayout),
vkGraphicsPipeline(graphicsPipeline),
swapChainFramebuffersObj(framebuffers),
vkCommandBuffers(commandBuffers),
uniformBuffersObjRef(uniformBuffers),
syncObjectsRef(syncObjects),
MAX_FRAMES_IN_FLIGHT_RENDERER(maxFramesInFlight),
currentFrame(0)
{
}

VulkanRenderer::~VulkanRenderer()
{
}

//void VulkanRenderer::drawFrame(
//    std::function<FrameUniformBufferObject()> uboDataProvider,
//    bool& framebufferResized,
//    std::function<void()> recreateSwapChainCallback,
//    const std::vector<RenderableObject>& renderables,
//    VkDeviceSize dynamicUboAlignment
//) {
//    VkFence currentFrameFence = syncObjectsRef.getInFlightFence(currentFrame);
//    vkWaitForFences(devices.getLogicalDevice(), 1, &currentFrameFence, VK_TRUE, UINT64_MAX);
//
//    uint32_t imageIndex;
//    VkResult result = vkAcquireNextImageKHR(
//        devices.getLogicalDevice(),
//        swapChainObj.getSwapChain(),
//        UINT64_MAX,
//        syncObjectsRef.getImageAvailableSemaphore(currentFrame),
//        VK_NULL_HANDLE,
//        &imageIndex
//    );
//
//    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//        recreateSwapChainCallback(); // Call the callback provided by HelloTriangleApplication
//        return;
//    }
//    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//        throw std::runtime_error("failed to acquire swap chain image!");
//    }
//
//    /*VkFence imageInFlightFence = syncObjectsRef.getImageInFlightFence(imageIndex);
//    if (imageInFlightFence != VK_NULL_HANDLE)
//    {
//        vkWaitForFences(devices.getLogicalDevice(), 1, &imageInFlightFence, VK_TRUE, UINT64_MAX);
//    }
//
//    syncObjectsRef.getImageInFlightFence(imageIndex) = currentFrameFence;*/
//
//
//
//    // Get UBO data using the provided function (contains view/proj)
//    FrameUniformBufferObject uboData = uboDataProvider();
//    uniformBuffersObjRef.update(currentFrame, uboData);
//
//    vkResetFences(devices.getLogicalDevice(), 1, &currentFrameFence);
//
//    VkCommandBuffer currentCmdBuffer = vkCommandBuffers.getCommandBuffer(currentFrame);
//    vkResetCommandBuffer(currentCmdBuffer, 0);
//
//    // Note: The static recordCommandBuffer might need to become a member of VulkanCommandBuffers
//    // or be passed all necessary handles if it remains static.
//    // For this example, assuming it can be called like this, or adapt as needed.
//    VulkanCommandBuffers::recordCommandBuffer( // Or make this a method of VulkanCommandBuffers instance
//        currentCmdBuffer,
//        currentFrame, // Pass currentFrame if descriptorSets are indexed by it directly
//        vkRenderPass.getVkRenderPass(),
//        vkPipelineLayout.getVkPipelineLayout(),
//        vkGraphicsPipeline.getVkPipeline(),
//        swapChainFramebuffersObj.getFramebuffer(imageIndex),
//        swapChainObj.getExtent(),
//        renderables,
//        dynamicUboAlignment
//    );
//
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//    VkSemaphore waitSemaphores[] = { syncObjectsRef.getImageAvailableSemaphore(currentFrame) };
//    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//    submitInfo.waitSemaphoreCount = 1;
//    submitInfo.pWaitSemaphores = waitSemaphores;
//    submitInfo.pWaitDstStageMask = waitStages;
//
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &currentCmdBuffer;
//
//    VkSemaphore signalSemaphores[] = { syncObjectsRef.getRenderFinishedSemaphore(currentFrame) };
//    submitInfo.signalSemaphoreCount = 1;
//    submitInfo.pSignalSemaphores = signalSemaphores;
//
//    if (vkQueueSubmit(devices.getGraphicsQueue(), 1, &submitInfo, currentFrameFence) != VK_SUCCESS) {
//        throw std::runtime_error("failed to submit draw command buffer!");
//    }
//
//    VkPresentInfoKHR presentInfo{};
//    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    presentInfo.waitSemaphoreCount = 1;
//    presentInfo.pWaitSemaphores = signalSemaphores;
//    VkSwapchainKHR swapChains[] = { swapChainObj.getSwapChain() };
//    presentInfo.swapchainCount = 1;
//    presentInfo.pSwapchains = swapChains;
//    presentInfo.pImageIndices = &imageIndex;
//
//    result = vkQueuePresentKHR(devices.getPresentQueue(), &presentInfo);
//
//    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
//        framebufferResized = false; // Reset the flag here, as renderer is handling it
//        recreateSwapChainCallback();
//    }
//    else if (result != VK_SUCCESS) {
//        throw std::runtime_error("failed to present swap chain image!");
//    }
//
//    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT_RENDERER;
//}

void VulkanRenderer::drawFrame(
    std::function<FrameUniformBufferObject()> uboDataProvider,
    bool& framebufferResized,
    std::function<void()> recreateSwapChainCallback,
    const std::vector<RenderableObject>& renderables,
    VkDeviceSize dynamicUboAlignment
)
{
    // 1. Wait for the fence of the current frame to ensure the GPU is done with it.
    VkFence currentFrameFence = syncObjectsRef.getInFlightFence(currentFrame);
    vkWaitForFences(devices.getLogicalDevice(), 1, &currentFrameFence, VK_TRUE, UINT64_MAX);

    // 2. Acquire an image from the swap chain.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(devices.getLogicalDevice(), swapChainObj.getSwapChain(), UINT64_MAX, syncObjectsRef.getImageAvailableSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChainCallback();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Update the uniform buffer for this frame
    FrameUniformBufferObject ubo = uboDataProvider();
    uniformBuffersObjRef.update(currentFrame, ubo);

    // 3. We are about to use this frame's command buffer, so reset its fence.
    vkResetFences(devices.getLogicalDevice(), 1, &currentFrameFence);

    // 4. Record the command buffer.
    VkCommandBuffer currentCmdBuffer = vkCommandBuffers.getCommandBuffer(currentFrame);
    vkResetCommandBuffer(currentCmdBuffer, 0);
    VulkanCommandBuffers::recordCommandBuffer( // Or make this a method of VulkanCommandBuffers instance
        currentCmdBuffer,
        currentFrame, // Pass currentFrame if descriptorSets are indexed by it directly
        vkRenderPass.getVkRenderPass(),
        vkPipelineLayout.getVkPipelineLayout(),
        vkGraphicsPipeline.getVkPipeline(),
        swapChainFramebuffersObj.getFramebuffer(imageIndex),
        swapChainObj.getExtent(),
        renderables,
        dynamicUboAlignment
    );

    // 5. Submit the command buffer to the graphics queue.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { syncObjectsRef.getImageAvailableSemaphore(currentFrame) };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCmdBuffer;

    VkSemaphore signalSemaphores[] = { syncObjectsRef.getRenderFinishedSemaphore(currentFrame) };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(devices.getGraphicsQueue(), 1, &submitInfo, syncObjectsRef.getInFlightFence(currentFrame)) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 6. Present the image to the screen.
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Wait on the semaphore we just signaled

    VkSwapchainKHR swapChains[] = { swapChainObj.getSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(devices.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChainCallback();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // 7. Advance to the next frame. This is crucial for using the correct sync objects next time.
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT_RENDERER;
}