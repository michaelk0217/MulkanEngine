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

void VulkanRenderer::recordSceneCommands(
    VkCommandBuffer commandBuffer, 
    const RenderPacket& packet, 
    uint32_t currentFrameIndex,
    VkFramebuffer swapChainFramebuffer)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass.getVkRenderPass();
    renderPassInfo.framebuffer = swapChainFramebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainObj.getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainObj.getExtent().width);
    viewport.height = static_cast<float>(swapChainObj.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainObj.getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (packet.skyboxData.has_value() && packet.skyboxData->renderSkyBox)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.skyboxData->pipeline);
        VkBuffer vertexBuffers[] = { packet.skyboxData->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        VkDescriptorSet skyboxDescSet = packet.skyboxData->descriptorSets[currentFrameIndex];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.skyboxData->pipelineLayout, 0, 1, &skyboxDescSet, 0, nullptr);

        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
    }

    // --- draw pbr objects ---
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pbrPipeline);
    for (uint32_t i = 0; i < packet.pbrRenderables.size(); i++)
    {
        const auto& renderable = packet.pbrRenderables[i];

        if (!renderable.vertexBuffer || !renderable.indexBuffer) continue; // skip

        VkBuffer vertexBuffers[] = { renderable.vertexBuffer->getVkBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, renderable.indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

        uint32_t dynamicOffset = i * static_cast<uint32_t>(packet.dynamicUboAlignment);

        VkDescriptorSet materialDescriptorSet = renderable.material->frameSpecificDescriptorSets[currentFrameIndex];

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, packet.pbrLayout,
            0, 1, &materialDescriptorSet,
            1, &dynamicOffset);

        uint32_t useOrm = renderable.material->useOrm ? 1 : 0;
        vkCmdPushConstants(commandBuffer, packet.pbrLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &useOrm);

        vkCmdDrawIndexed(commandBuffer, renderable.indexCount, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
}

//void VulkanRenderer::recordCommandBuffer(
//    VkCommandBuffer commandBuffer,
//    uint32_t currentFrameIndex,
//    VkRenderPass renderPass,
//    VkPipelineLayout pipelineLayout,
//    VkPipeline graphicsPipeline,
//    VkFramebuffer swapChainFramebuffer,
//    VkExtent2D swapChainExtent,
//    const std::vector<RenderableObject>& renderables,
//    VkDeviceSize dynamicUboAlignment,
//    std::optional<SkyboxData> skyboxPacket
// )
//{
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.flags = 0; // Optional
//    beginInfo.pInheritanceInfo = nullptr; // Optional
//
//    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to begin recording command buffer!");
//    }
//
//    VkRenderPassBeginInfo renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//    renderPassInfo.renderPass = renderPass;
//    renderPassInfo.framebuffer = swapChainFramebuffer;
//    renderPassInfo.renderArea.offset = { 0, 0 };
//    renderPassInfo.renderArea.extent = swapChainExtent;
//
//    std::array<VkClearValue, 2> clearValues{};
//    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
//    clearValues[1].depthStencil = { 1.0f, 0 };
//    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
//    renderPassInfo.pClearValues = clearValues.data();
//
//    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//    VkViewport viewport{};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = static_cast<float>(swapChainExtent.width);
//    viewport.height = static_cast<float>(swapChainExtent.height);
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
//
//    VkRect2D scissor{};
//    scissor.offset = { 0, 0 };
//    scissor.extent = swapChainExtent;
//    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
//
//
//    // --- draw skybox --- if exists in packet
//    if (skyboxPacket.has_value() && skyboxPacket->renderSkyBox)
//    {
//        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPacket->pipeline);
//        VkBuffer vertexBuffers[] = { skyboxPacket->vertexBuffer };
//        VkDeviceSize offsets[] = { 0 };
//        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//        VkDescriptorSet skyboxDescSet = skyboxPacket->descriptorSets[currentFrameIndex];
//        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPacket->pipelineLayout, 0, 1, &skyboxDescSet, 0, nullptr);
//
//        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
//    }
//
//    
//    // --- draw pbr objects ---
//    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
//    for (uint32_t i = 0; i < renderables.size(); i++)
//    {
//        const auto& renderable = renderables[i];
//
//        if (!renderable.vertexBuffer || !renderable.indexBuffer) continue; // skip
//
//        VkBuffer vertexBuffers[] = { renderable.vertexBuffer->getVkBuffer() };
//        VkDeviceSize offsets[] = { 0 };
//        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//
//        vkCmdBindIndexBuffer(commandBuffer, renderable.indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
//
//        uint32_t dynamicOffset = i * static_cast<uint32_t>(dynamicUboAlignment);
//
//        VkDescriptorSet materialDescriptorSet = renderable.material->frameSpecificDescriptorSets[currentFrameIndex];
//
//        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
//            0, 1, &materialDescriptorSet,
//            1, &dynamicOffset);
//
//        uint32_t useOrm = renderable.material->useOrm ? 1 : 0;
//        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &useOrm);
//
//        vkCmdDrawIndexed(commandBuffer, renderable.indexCount, 1, 0, 0, 0);
//    }
//
//
//    vkCmdEndRenderPass(commandBuffer);
//
//    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to record command buffer!");
//    }
//}