#include "ImGuiManager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <glm/glm.hpp>

#include "Window.h"
#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapChain.h"
#include "VulkanCommandPool.h"
#include "VulkanRenderPass.h"
#include "VulkanUniformBuffers.h"
#include "VulkanGlobals.h"
#include "Renderable.h"

ImGuiManager::ImGuiManager(
	Window& window, 
	VulkanInstance& instance, 
	VulkanDevice& device,
    VulkanSurface& surface,
	VulkanSwapChain& swapchain, 
	VulkanCommandPool& commandPool) :
	m_device(device)
{

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(m_device.getLogicalDevice(), &pool_info, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool for ImGui!");
    }

    initRenderPass(swapchain.getImageFormat());

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window.getGlfwWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance.getVkInstance();
    init_info.PhysicalDevice = device.getPhysicalDevice();
    init_info.Device = device.getLogicalDevice();
    init_info.RenderPass = m_renderPass;
    init_info.QueueFamily = findQueueFamilies(device.getPhysicalDevice(), surface.getVkSurface()).graphicsFamily.value();
    init_info.Queue = device.getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = VulkanGlobals::MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = static_cast<uint32_t>(swapchain.getImageViews().size());
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result; // You can add a validation check function here

    ImGui_ImplVulkan_Init(&init_info);
    
}

ImGuiManager::~ImGuiManager()
{
    vkDeviceWaitIdle(m_device.getLogicalDevice());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyRenderPass(m_device.getLogicalDevice(), m_renderPass, nullptr);
    vkDestroyDescriptorPool(m_device.getLogicalDevice(), m_descriptorPool, nullptr);
}

void ImGuiManager::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}



void ImGuiManager::buildUI(SceneDebugContextPacket& sceneDebugContextPacket)
{
    drawControlPanel(sceneDebugContextPacket);
    drawLightingPanel(sceneDebugContextPacket);
}

void ImGuiManager::render(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D swapChainExtent)
{
    ImGui::Render();

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = m_renderPass;
    info.framebuffer = framebuffer;
    info.renderArea.extent = swapChainExtent;
    info.renderArea.offset = { 0, 0 };
    info.clearValueCount = 0;
    info.pClearValues = nullptr;

    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
}



void ImGuiManager::check_vk_result(VkResult err)
{
    if (err == 0)
    {
        return;
    }
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
    {
        abort();
    }
}

VkRenderPass ImGuiManager::getRenderPass() const
{
    return m_renderPass;
}

void ImGuiManager::initRenderPass(VkFormat swapChainFormat)
{
    
    VkAttachmentDescription attachment = {};
    attachment.format = swapChainFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device.getLogicalDevice(), &info, nullptr, &m_renderPass) != VK_SUCCESS) { // You might need a way to set the handle in your class
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }
}

void ImGuiManager::drawControlPanel(SceneDebugContextPacket& sceneDebugContextPacket)
{
    ImGui::Begin("Engine Controls");
    ImGui::Checkbox("Wireframe Mode", &sceneDebugContextPacket.wireframeMode);
    //ImGui::Text("Tessellation Level: %.1f", tessUboData.tessellationLevel);
    ImGui::SliderFloat("Tessellation Level", &sceneDebugContextPacket.tessellationUbo.tessellationLevel, 1.0f, 64.0f);
    ImGui::SliderFloat("Displacement Scale", &sceneDebugContextPacket.tessellationUbo.displacementScale, 0.0f, 0.2f);
    ImGui::End();
}

void ImGuiManager::drawLightingPanel(SceneDebugContextPacket& sceneDebugContextPacket)
{
    ImGui::Begin("Lighting Controls");
    ImGui::Text("Directional Light");
    // We pass a pointer to the start of the data and ImGui treats it as an array of 3 floats
    ImGui::DragFloat3("Direction", &sceneDebugContextPacket.sceneLighingUbo.dirLight.direction.x, 0.01f);
    // Normalize the direction after modification
    sceneDebugContextPacket.sceneLighingUbo.dirLight.direction = glm::normalize(sceneDebugContextPacket.sceneLighingUbo.dirLight.direction);

    ImGui::ColorEdit3("Color", &sceneDebugContextPacket.sceneLighingUbo.dirLight.color.r);
    ImGui::DragFloat("Intensity", &sceneDebugContextPacket.sceneLighingUbo.dirLight.color.w, 0.1f, 0.0f, 100.0f); // 'w' is intensity

    ImGui::End();
}
