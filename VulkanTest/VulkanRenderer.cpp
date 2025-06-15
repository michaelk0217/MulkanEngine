#include "VulkanRenderer.h"
#include <stdexcept> // For runtime_error
#include <iostream>  // For debug/error output

VulkanRenderer::VulkanRenderer(
    VulkanDevice& device,
    VulkanSwapChain& swapChain,
    VulkanRenderPass& renderPass,
    VulkanFramebuffers& framebuffers,
    VulkanCommandBuffers& commandBuffers,
    VulkanSyncObjects& syncObjects,
    int maxFramesInFlight
) : devices(device),
swapChainObj(swapChain),
vkRenderPass(renderPass),
swapChainFramebuffersObj(framebuffers),
vkCommandBuffers(commandBuffers),
syncObjectsRef(syncObjects),
MAX_FRAMES_IN_FLIGHT_RENDERER(maxFramesInFlight),
currentFrame(0)
{
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::drawFrame(
    RenderPacket& packet,
    bool& framebufferResized,
    std::function<void()> recreateSwapChainCallback
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

    // 3. We are about to use this frame's command buffer, so reset its fence.
    vkResetFences(devices.getLogicalDevice(), 1, &currentFrameFence);

    // 4. Record the command buffer.
    VkCommandBuffer currentCmdBuffer = vkCommandBuffers.getCommandBuffer(currentFrame);
    vkResetCommandBuffer(currentCmdBuffer, 0);
    recordCommandBuffer( // Or make this a method of VulkanCommandBuffers instance
        currentCmdBuffer,
        currentFrame, // Pass currentFrame if descriptorSets are indexed by it directly
        vkRenderPass.getVkRenderPass(),
        packet.pbrLayout,
        packet.pbrPipeline,
        swapChainFramebuffersObj.getFramebuffer(imageIndex),
        swapChainObj.getExtent(),
        packet.pbrRenderables,
        packet.dynamicUboAlignment,
        packet.skyboxData->pipeline,
        packet.skyboxData->vertexBuffer,
        packet.skyboxData->descriptorSets,
        packet.skyboxData->pipelineLayout
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

void VulkanRenderer::recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t currentFrameIndex,
    VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    VkPipeline graphicsPipeline,
    VkFramebuffer swapChainFramebuffer,
    VkExtent2D swapChainExtent,
    const std::vector<RenderableObject>& renderables,
    VkDeviceSize dynamicUboAlignment,
    VkPipeline skyboxGraphicsPipeline,
    VkBuffer skyboxVertexBuffer,
    std::vector<VkDescriptorSet> skyboxDescriptorSets,
    VkPipelineLayout skyboxPipelineLayout)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


    // --- draw skybox --- if exists in packet
    if (skyboxGraphicsPipeline != VK_NULL_HANDLE && !skyboxDescriptorSets.empty() && skyboxVertexBuffer != VK_NULL_HANDLE && skyboxPipelineLayout != VK_NULL_HANDLE)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxGraphicsPipeline);
        VkBuffer vertexBuffers[] = { skyboxVertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        VkDescriptorSet skyboxDescSet = skyboxDescriptorSets[currentFrameIndex];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipelineLayout, 0, 1, &skyboxDescSet, 0, nullptr);

        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
    }

    
    // --- draw pbr objects ---
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    for (uint32_t i = 0; i < renderables.size(); i++)
    {
        const auto& renderable = renderables[i];

        if (!renderable.vertexBuffer || !renderable.indexBuffer) continue; // skip

        VkBuffer vertexBuffers[] = { renderable.vertexBuffer->getVkBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, renderable.indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

        uint32_t dynamicOffset = i * static_cast<uint32_t>(dynamicUboAlignment);

        VkDescriptorSet materialDescriptorSet = renderable.material->frameSpecificDescriptorSets[currentFrameIndex];

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
            0, 1, &materialDescriptorSet,
            1, &dynamicOffset);

        vkCmdDrawIndexed(commandBuffer, renderable.indexCount, 1, 0, 0, 0);
    }


    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}