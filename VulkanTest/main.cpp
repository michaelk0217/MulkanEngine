//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <chrono>
#include <memory>
#include <map>
#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include "VulkanGlobals.h"
#include "Window.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFramebuffers.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffers.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanUniformBuffers.h"
#include "VulkanTexture.h"
#include "VulkanDepthResources.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSets.h"
#include "VulkanSyncObjects.h"
#include "VulkanRenderer.h"
#include "ModelLoader.h"
#include "utils.h"

#include "Camera.h"
#include "Renderable.h"
#include "Material.h"
#include "Lights.h"
#include "AssetManager.h"
#include "ImGuiManager.h"


const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}


class VulkanEngine
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	std::unique_ptr<Window> window;

	std::unique_ptr<VulkanInstance> instance;

	std::unique_ptr<VulkanSurface> surface;

	std::unique_ptr<VulkanDevice> devices;

	std::unique_ptr<VulkanSwapChain> swapChainObj;

	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanDescriptorPool> descriptorPool;

	std::unique_ptr<VulkanDescriptorSetLayout> m_pbrDescriptorSetLayout;
	//std::unique_ptr<VulkanDescriptorSets> m_pbrDescriptorSets;
	std::unique_ptr<VulkanPipelineLayout> m_pbrPipelineLayout;

	std::unique_ptr<VulkanDescriptorSetLayout> m_skyboxDescriptorSetLayout;
	std::unique_ptr<VulkanDescriptorSets> m_skyboxDescriptorSets;
	std::unique_ptr<VulkanPipelineLayout> m_skyboxPipelineLayout;
	std::unique_ptr<VulkanVertexBuffer> m_skyboxCubeVertexBuffer;

	//std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelineFill;
	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelinePBR;
	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelineWireframe;
	bool m_WireframeMode = false;
	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelineSkybox;

	std::unique_ptr<VulkanFramebuffers> swapChainFramebuffers;

	std::unique_ptr<VulkanCommandPool> commandPool;
	std::unique_ptr<VulkanCommandBuffers> commandBuffers;

	std::unique_ptr<VulkanSyncObjects> syncObjects;

	bool framebufferResized = false;

	std::unique_ptr<VulkanDepthResources> depthResourceObj;

	std::unique_ptr<VulkanRenderer> renderer;

	std::unique_ptr<Camera> camera;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	float deltaTime = 0.0f;
	float accumulatedTime = 0.0f;


	// --- Collection of Model resources --- 
	std::unique_ptr<AssetManager> m_AssetManager;

	std::vector<RenderableObject> renderableObjects;

	std::unique_ptr<VulkanTexture> skyboxTexture;
	std::unique_ptr<VulkanTexture> irradianceMap;
	std::unique_ptr<VulkanTexture> prefilterMap;
	std::unique_ptr<VulkanTexture> brdfLut;

	// UniformBuffers
	std::unique_ptr<VulkanUniformBuffers> frameUboManager;
	std::unique_ptr<VulkanUniformBuffers> objectDataDUBManager;
	std::unique_ptr<VulkanUniformBuffers> lightingUboManager;
	std::unique_ptr<VulkanUniformBuffers> materialUboManager;
	SceneLightingUBO sceneLights{}; // CPU SIDE DATA
	//std::unique_ptr<VulkanUniformBuffers> tessellationUboManager;
	//TessellationUBO tessUboData; // CPU SIDE DATA
	size_t tessLevelIndex = 0;
	std::array<float, 5> tessLevelValue{ 1.0f, 4.0f, 8.0f, 16.0f, 64.0f };
	// -------------------------------------

	// IMGUI
	std::unique_ptr<ImGuiManager> m_imguiManager;
	std::unique_ptr<VulkanFramebuffers> m_imguiFramebuffers;

	void initWindow()
	{

		const uint32_t WIDTH_CONST = 2560;
		const uint32_t HEIGHT_CONST = 1440;
		const std::string TITLE = "Mulkan Engine";
		try
		{
			window = std::make_unique<Window>(WIDTH_CONST, HEIGHT_CONST, TITLE);

			window->setAppFramebufferResizeCallback([this](int width, int height)
				{
					// 'this' is HelloTriangleApplilcation* captured by the lambda
					this->framebufferResized = true;
				}
			);

		}
		catch (const std::runtime_error& e)
		{
			std::cerr << "Window Initialization Error: " << e.what() << std::endl;
			throw;
		}
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void initVulkan()
	{
		instance = std::make_unique<VulkanInstance>();
		instance->createInstance();
		instance->setupDebugMessenger(); // Debug

		surface = std::make_unique<VulkanSurface>();
		surface->createSurface(instance->getVkInstance(), window->getGlfwWindow());

		devices = std::make_unique<VulkanDevice>();
		devices->createDevices(instance->getVkInstance(), surface->getVkSurface(), deviceExtensions, validationLayers);

		swapChainObj = std::make_unique<VulkanSwapChain>();
		swapChainObj->create(devices->getPhysicalDevice(), devices->getLogicalDevice(), surface->getVkSurface(), window->getGlfwWindow());

		renderPass = std::make_unique<VulkanRenderPass>();
		renderPass->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), swapChainObj->getImageFormat());

		m_pbrDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>();
		m_pbrDescriptorSetLayout->create(devices->getLogicalDevice());

		/*VkPushConstantRange pbrPipelinePushConstantRange{};
		pbrPipelinePushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pbrPipelinePushConstantRange.offset = 0;
		pbrPipelinePushConstantRange.size = sizeof(uint32_t);*/
		m_pbrPipelineLayout = std::make_unique<VulkanPipelineLayout>();
		m_pbrPipelineLayout->create(devices->getLogicalDevice(), m_pbrDescriptorSetLayout->getVkDescriptorSetLayout());

		m_skyboxDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>();
		m_skyboxDescriptorSetLayout->createForSkybox(devices->getLogicalDevice());

		m_skyboxPipelineLayout = std::make_unique<VulkanPipelineLayout>();
		m_skyboxPipelineLayout->create(devices->getLogicalDevice(), m_skyboxDescriptorSetLayout->getVkDescriptorSetLayout());

		// --- Graphics Pipieline ---
		//m_GraphicsPipelineFill = std::make_unique<VulkanGraphicsPipeline>();
		//m_GraphicsPipelineFill->create(
		//	devices->getLogicalDevice(), 
		//	m_pbrPipelineLayout->getVkPipelineLayout(),
		//	renderPass->getVkRenderPass(), 
		//	"shaders/vert.spv", 
		//	"shaders/frag.spv",
		//	//"shaders/tess.tesc.spv",
		//	//"shaders/tess.tese.spv",
		//	VK_POLYGON_MODE_FILL,
		//	VK_CULL_MODE_BACK_BIT,
		//	VK_FRONT_FACE_CLOCKWISE
		//);
		m_GraphicsPipelinePBR = std::make_unique<VulkanGraphicsPipeline>();
		m_GraphicsPipelinePBR->create(
			devices->getLogicalDevice(),
			m_pbrPipelineLayout->getVkPipelineLayout(),
			renderPass->getVkRenderPass(),
			"shaders/vert.spv",
			"shaders/frag.spv",
			//"shaders/tess.tesc.spv",
			//"shaders/tess.tese.spv",
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			//VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_COUNTER_CLOCKWISE
		);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(devices->getPhysicalDevice(), &deviceFeatures);
		if (deviceFeatures.fillModeNonSolid)
		{
			m_GraphicsPipelineWireframe = std::make_unique<VulkanGraphicsPipeline>();
			m_GraphicsPipelineWireframe->create(
				devices->getLogicalDevice(),
				m_pbrPipelineLayout->getVkPipelineLayout(),
				renderPass->getVkRenderPass(),
				"shaders/vert.spv",
				"shaders/wireframe.frag.spv",
				//"shaders/tess.tesc.spv",
				//"shaders/tess.tese.spv",
				VK_POLYGON_MODE_LINE,
				VK_CULL_MODE_BACK_BIT,
				VK_FRONT_FACE_COUNTER_CLOCKWISE
			);
		}

		// Skybox Graphics Pipeline
		m_GraphicsPipelineSkybox = std::make_unique<VulkanGraphicsPipeline>();
		m_GraphicsPipelineSkybox->createSkybox(
			devices->getLogicalDevice(),
			m_skyboxPipelineLayout->getVkPipelineLayout(),
			renderPass->getVkRenderPass(),
			"shaders/skybox.vert.spv",
			"shaders/skybox.frag.spv"
		);
		// ---------------------------

		commandPool = std::make_unique<VulkanCommandPool>();
		commandPool->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), surface->getVkSurface());

		depthResourceObj = std::make_unique<VulkanDepthResources>();
		depthResourceObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), swapChainObj->getExtent());

		swapChainFramebuffers = std::make_unique<VulkanFramebuffers>();
		swapChainFramebuffers->create(devices->getLogicalDevice(), swapChainObj->getImageViews(), depthResourceObj->getDepthImageView(), renderPass->getVkRenderPass(), swapChainObj->getExtent());

		// --- assets ---

		m_AssetManager = std::make_unique<AssetManager>(devices.get(), commandPool.get());


		loadAssetsAndCreateRenderables();
		

		// --- 1. Define Counts ---
		uint32_t materialCount = static_cast<uint32_t>(m_AssetManager->getMaterials().size());
		uint32_t framesInFlight = VulkanGlobals::MAX_FRAMES_IN_FLIGHT;
		const uint32_t SAMPLERS_PER_PBR_SET = 8; // Because your layout still has the separate occlusion sampler

		// --- 2. Calculate Per-Category Set Counts ---
		uint32_t pbrMaterialSets = materialCount * framesInFlight;
		uint32_t skyboxSets = framesInFlight;
		uint32_t iblConversionSets = 3;
		uint32_t totalSets = pbrMaterialSets + skyboxSets + iblConversionSets;

		// --- 3. Calculate Total Per-Type Descriptor Needs ---
		uint32_t totalUbos = (3 * pbrMaterialSets) + (1 * skyboxSets) + 2; // PBR UBOs + Skybox UBO + 2 for IBL gens
		uint32_t totalDynamicUbos = 1 * pbrMaterialSets;                  // PBR Dynamic UBO
		uint32_t totalSamplers = (SAMPLERS_PER_PBR_SET * pbrMaterialSets) + (1 * skyboxSets) + 3; // PBR Samplers + Skybox Sampler + 3 for IBL gens

		// --- 4. Create the Pool Size Vector ---
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, totalUbos },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, totalDynamicUbos },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, totalSamplers }
		};
	
		descriptorPool = std::make_unique<VulkanDescriptorPool>(); // load descriptor pool after populating renderableObjects
		// materials * frames (maxsets) + skybox * frames + 3 conversion sets
		descriptorPool->create(devices->getLogicalDevice(), totalSets, poolSizes);

		auto hdrSourceTexture = std::make_unique <VulkanTexture>();
		hdrSourceTexture->createTextureHDR(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), "textures/skybox/kloppenheim_06_puresky_4k.hdr");

		const uint32_t cubemapSize = 1024;
		skyboxTexture = std::make_unique<VulkanTexture>();
		skyboxTexture->createCubemap(devices->getLogicalDevice(), devices->getPhysicalDevice(), cubemapSize, cubemapSize, 1);

		loadCubeModel(); // loads m_skyboxCubeBuffer

		generateSkyboxCubeMap(*hdrSourceTexture, *skyboxTexture, cubemapSize);

		hdrSourceTexture->destroy();
		hdrSourceTexture.reset();

		// --- uniform buffers ---
		frameUboManager = std::make_unique<VulkanUniformBuffers>();
		frameUboManager->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, sizeof(FrameUniformBufferObject));

		objectDataDUBManager = std::make_unique<VulkanUniformBuffers>();
		objectDataDUBManager->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT,
			VulkanUniformBuffers::totalObjectDataBufferSize(devices->getPhysicalDevice()),
			true);

		lightingUboManager = std::make_unique<VulkanUniformBuffers>();
		lightingUboManager->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, sizeof(SceneLightingUBO));

		sceneLights.dirLight.direction = glm::normalize(glm::vec4(-0.5, -1.0f, -0.5f, 0.0f));
		sceneLights.dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 10.0f); //w intensity

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(devices->getPhysicalDevice(), &properties);
		size_t minUboAlignment = properties.limits.minUniformBufferOffsetAlignment;
		size_t alignedMaterialUboSize = getAlignedUboSize(sizeof(MaterialUBO), minUboAlignment);
		VkDeviceSize totalMaterialUboSize = alignedMaterialUboSize * m_AssetManager->getMaterials().size();
		materialUboManager = std::make_unique<VulkanUniformBuffers>();
		VkDeviceSize materialUboSize = sizeof(MaterialUBO) * m_AssetManager->getMaterials().size();
		materialUboManager->create(
			devices->getLogicalDevice(),
			devices->getPhysicalDevice(),
			VulkanGlobals::MAX_FRAMES_IN_FLIGHT,
			//materialUboSize,
			totalMaterialUboSize,
			false
		);
		// populate materialUbo
		int materialIndex = 0;
		for (auto const& [name, material] : m_AssetManager->getMaterials())
		{
			for (int frame = 0; frame < VulkanGlobals::MAX_FRAMES_IN_FLIGHT; ++frame)
			{
				char* mappedData = static_cast<char*>(materialUboManager->getMappedMemory(frame));
				memcpy(mappedData + (materialIndex * alignedMaterialUboSize), &material->uboData, sizeof(MaterialUBO));
			}
			materialIndex++;
		}

		//tessellationUboManager = std::make_unique<VulkanUniformBuffers>();
		//tessellationUboManager->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, sizeof(TessellationUBO));

		// ------------------------

		generateIrradianceMap(); // generates irradianceMap of skybox & requires frameUboManager to be initialized
		generatePrefilerMap(); // generates prefilterMap of skybox
		generateBrdfLut();

		IblPacket iblPacket{};
		iblPacket.irradianceImageView = irradianceMap->getImageView();
		iblPacket.irradianceSampler = irradianceMap->getSampler();
		iblPacket.prefilterImageView = prefilterMap->getImageView();
		iblPacket.prefilterSampler = prefilterMap->getSampler();
		iblPacket.brdfLutImageView = brdfLut->getImageView();
		iblPacket.brdfLutSampler = brdfLut->getSampler();

		//m_pbrDescriptorSets = std::make_unique<VulkanDescriptorSets>();
		VulkanDescriptorSets::createForMaterials(
			devices->getLogicalDevice(),
			descriptorPool->getVkDescriptorPool(),
			m_pbrDescriptorSetLayout->getVkDescriptorSetLayout(),
			VulkanGlobals::MAX_FRAMES_IN_FLIGHT,
			frameUboManager->getBuffers(),
			objectDataDUBManager->getBuffers(),
			lightingUboManager->getBuffers(),
			materialUboManager->getBuffers(),
			m_AssetManager->getMaterials(),
			iblPacket,
			alignedMaterialUboSize
		);

		m_skyboxDescriptorSets = std::make_unique<VulkanDescriptorSets>();
		m_skyboxDescriptorSets->createForSkybox(
			devices->getLogicalDevice(),
			descriptorPool->getVkDescriptorPool(),
			m_skyboxDescriptorSetLayout->getVkDescriptorSetLayout(),
			VulkanGlobals::MAX_FRAMES_IN_FLIGHT,
			frameUboManager->getBuffers(),
			*skyboxTexture
		);


		commandBuffers = std::make_unique<VulkanCommandBuffers>();
		commandBuffers->create(devices->getLogicalDevice(), commandPool->getVkCommandPool(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT);

		syncObjects = std::make_unique<VulkanSyncObjects>();
		syncObjects->create(devices->getLogicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(swapChainObj->getImageViews().size()));

		m_imguiManager = std::make_unique<ImGuiManager>(
			*window,
			*instance,
			*devices,
			*surface,
			*swapChainObj,
			*commandPool
		);
		m_imguiFramebuffers = std::make_unique<VulkanFramebuffers>();
		m_imguiFramebuffers->createForImGui(devices->getLogicalDevice(), swapChainObj->getImageViews(), m_imguiManager->getRenderPass(), swapChainObj->getExtent());

		renderer = std::make_unique<VulkanRenderer>(
			*devices,            // Pass by reference
			*swapChainObj,
			*renderPass,
			//*m_pbrPipelineLayout,
			*swapChainFramebuffers,
			*commandBuffers,
			*syncObjects,
			VulkanGlobals::MAX_FRAMES_IN_FLIGHT
		);

		camera = std::make_unique<Camera>(
			glm::vec3(2.0f, 2.0f, 2.0f),       // startPosition
			glm::vec3(0.0f, 1.0f, 0.0f),       // worldUpVector: SET TO Y-UP
			-90.0f,                            // startYaw (e.g., -90 to look along -Z, or 0 to look along +X)
			// Your previous 45.0f is also fine, just be aware of direction.
			0.0f,                              // startPitch
			5.0f,                              // startMoveSpeed
			0.1f,                              // startTurnSpeed
			45.0f,                             // fieldOfView
			swapChainObj->getExtent().width / (float)swapChainObj->getExtent().height, // aspectRatioF
			0.1f,                              // nearF
			500.0f                              // farF
		);
	}

	void mainLoop()
	{
		startTime = std::chrono::high_resolution_clock::now();
		accumulatedTime = 0.0f;

		bool m_M_KeyPressed = false;
		bool up_KeyPressed = false;
		bool down_KeyPressed = false;
		

		while (window && !window->shouldClose())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = currentTime;
			accumulatedTime += deltaTime;

			window->pollEvents();

			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureKeyboard && !io.WantCaptureMouse)
			{
				processInput(deltaTime);
			}
			m_imguiManager->newFrame();
			//m_imguiManager->buildUI(m_WireframeMode, tessUboData);

			uint32_t uboFrameIndex = renderer->getCurrentFrame();

			//tessUboData.displacementScale = 0.0f;
			//tessellationUboManager->update(uboFrameIndex, tessUboData);
			frameUboManager->update(uboFrameIndex, frameUboUpdate());

			sceneLights.viewPosition = glm::vec4(camera->getCameraPosition(), 1.0f);
			lightingUboManager->update(uboFrameIndex, sceneLights);

			updateObjectUniforms(uboFrameIndex);

			VkPipeline pipelineToUse = m_WireframeMode
				? m_GraphicsPipelineWireframe->getVkPipeline()
				: m_GraphicsPipelinePBR->getVkPipeline();

			SkyboxData skyboxDataPacket{};
			skyboxDataPacket.pipeline = m_GraphicsPipelineSkybox->getVkPipeline();
			skyboxDataPacket.pipelineLayout = m_skyboxPipelineLayout->getVkPipelineLayout();
			skyboxDataPacket.vertexBuffer = m_skyboxCubeVertexBuffer->getVkBuffer();
			skyboxDataPacket.descriptorSets = m_skyboxDescriptorSets->getVkDescriptorSets();
			skyboxDataPacket.renderSkyBox = !m_WireframeMode;

			RenderPacket renderPacket{};
			renderPacket.pbrPipeline = pipelineToUse;
			//renderPacket.pbrPipeline_doubleSided = m_GraphicsPipeline_doubleSided->getVkPipeline();
			renderPacket.pbrLayout = m_pbrPipelineLayout->getVkPipelineLayout();
			renderPacket.pbrRenderables = renderableObjects;
			renderPacket.dynamicUboAlignment = objectDataDUBManager->getDynamicAlignment();
			renderPacket.skyboxData = skyboxDataPacket;

			SceneDebugContextPacket debugContextPacket
			{
				m_WireframeMode,
				//tessUboData,
				sceneLights,
				camera.get(),
				deltaTime,
				renderableObjects
			};

			m_imguiManager->buildUI(debugContextPacket);
			drawFrame(renderPacket);
			window->endFrame();
		}

		if (devices->isInitialized())
		{
			vkDeviceWaitIdle(devices->getLogicalDevice());
		}
	}

	void drawFrame(RenderPacket& renderPacket)
	{
		uint32_t frameIndex = renderer->getCurrentFrame();
		VkFence currentFrameFence = syncObjects->getInFlightFence(frameIndex);
		vkWaitForFences(devices->getLogicalDevice(), 1, &currentFrameFence, VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			devices->getLogicalDevice(),
			swapChainObj->getSwapChain(),
			UINT64_MAX,
			syncObjects->getImageAvailableSemaphore(frameIndex),
			VK_NULL_HANDLE,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized)
		{
			framebufferResized = false;
			recreateSwapChain();
			return; // continue; (should restart the main loop)
		} 
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(devices->getLogicalDevice(), 1, &currentFrameFence);

		VkCommandBuffer cmd = commandBuffers->getCommandBuffer(frameIndex);
		vkResetCommandBuffer(cmd, 0);

		// --- Begin Recording ---
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// SCENE ARTIST (draw scene)
		renderer->recordSceneCommands(
			cmd, 
			renderPacket, 
			frameIndex, 
			swapChainFramebuffers->getFramebuffer(imageIndex)
		);
		// UI ARTIST (draws imgui UI)
		m_imguiManager->render(
			cmd,
			m_imguiFramebuffers->getFramebuffer(imageIndex),
			swapChainObj->getExtent()
		);

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
		// --- Finished recording ---

		// -- Submit to GPU --
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { syncObjects->getImageAvailableSemaphore(frameIndex) };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		VkSemaphore signalSemaphores[] = { syncObjects->getRenderFinishedSemaphore(frameIndex) };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(devices->getGraphicsQueue(), 1, &submitInfo, currentFrameFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // Wait on the semaphore we just signaled

		VkSwapchainKHR swapChains[] = { swapChainObj->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(devices->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		renderer->advanceFrame();
	}

	void recreateSwapChain()
	{
		window->waitForRestoredSize();

		vkDeviceWaitIdle(devices->getLogicalDevice());

		cleanupSwapChain();

		swapChainObj->create(devices->getPhysicalDevice(), devices->getLogicalDevice(), surface->getVkSurface(), window->getGlfwWindow());
		depthResourceObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), swapChainObj->getExtent());
		swapChainFramebuffers->create(devices->getLogicalDevice(), swapChainObj->getImageViews(), depthResourceObj->getDepthImageView(), renderPass->getVkRenderPass(), swapChainObj->getExtent());

		m_imguiManager = std::make_unique<ImGuiManager>(
			*window,
			*instance,
			*devices,
			*surface,
			*swapChainObj,
			*commandPool
		);
		m_imguiFramebuffers->createForImGui(devices->getLogicalDevice(), swapChainObj->getImageViews(), m_imguiManager->getRenderPass(), swapChainObj->getExtent());
	}

	void cleanupSwapChain()
	{
		if (m_imguiManager) m_imguiManager.reset();

		depthResourceObj->destroy();

		swapChainFramebuffers->destroy();
		m_imguiFramebuffers->destroy();

		swapChainObj->destroy();
	}

	void cleanup()
	{
		vkDeviceWaitIdle(devices->getLogicalDevice());

		//m_imguiManager.reset();

		if (camera) camera.reset();

		cleanupSwapChain();

		if (renderer) renderer.reset();

		for (auto rendrableObj : renderableObjects)
		{
		}
		renderableObjects.clear();

		if (m_AssetManager) m_AssetManager.reset();

		if (skyboxTexture) skyboxTexture->destroy();
		skyboxTexture.reset();

		if (irradianceMap) irradianceMap->destroy();
		irradianceMap.reset();

		if (prefilterMap) prefilterMap->destroy();
		prefilterMap.reset();

		if (brdfLut) brdfLut->destroy();
		brdfLut.reset();

		if (m_skyboxCubeVertexBuffer) m_skyboxCubeVertexBuffer->destroy();
		m_skyboxCubeVertexBuffer.reset();

		if (frameUboManager) frameUboManager->destroy();
		frameUboManager.reset();

		if (objectDataDUBManager) objectDataDUBManager->destroy();
		objectDataDUBManager.reset();

		if (lightingUboManager) lightingUboManager->destroy();
		lightingUboManager.reset();

		if (materialUboManager) materialUboManager->destroy();
		materialUboManager.reset();

		//if (tessellationUboManager) tessellationUboManager->destroy();
		//tessellationUboManager.reset();

		if (syncObjects) syncObjects->destroy();
		syncObjects.reset();

	/*	if (m_pbrDescriptorSets) m_pbrDescriptorSets->destroy();
		m_pbrDescriptorSets.reset();*/

		if (descriptorPool) descriptorPool->destroy();
		descriptorPool.reset();

	/*	if (m_GraphicsPipelineFill) m_GraphicsPipelineFill->destroy();
		m_GraphicsPipelineFill.reset();*/

		if (m_GraphicsPipelinePBR) m_GraphicsPipelinePBR->destroy();
		m_GraphicsPipelinePBR.reset();

		if (m_GraphicsPipelineWireframe) m_GraphicsPipelineWireframe->destroy();
		m_GraphicsPipelineWireframe.reset();

		if (m_GraphicsPipelineSkybox) m_GraphicsPipelineSkybox->destroy();
		m_GraphicsPipelineSkybox.reset();

		if (m_pbrPipelineLayout) m_pbrPipelineLayout->destroy();
		m_pbrPipelineLayout.reset();

		if (m_skyboxPipelineLayout) m_skyboxPipelineLayout->destroy();
		m_skyboxPipelineLayout.reset();

		if (m_pbrDescriptorSetLayout) m_pbrDescriptorSetLayout->destroy();
		m_pbrDescriptorSetLayout.reset();

		if (m_skyboxDescriptorSetLayout) m_skyboxDescriptorSetLayout->destroy();
		m_skyboxDescriptorSetLayout.reset();

		if (renderPass) renderPass->destroy();
		renderPass.reset();

		if (commandBuffers) commandBuffers.reset();

		if (commandPool) commandPool->destroy();
		commandPool.reset();

		depthResourceObj.reset();
		swapChainFramebuffers.reset();
		swapChainObj.reset();

		if (devices) devices.reset();
		
		if (surface) surface.reset();

		if (instance) instance.reset();

		if (window) window.reset();
	}

	void processInput(float deltaTime)
	{
		// hold middle mouse button to enable cam movenet (mouse look & wasd movement)
		if (window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			camera->processKeyboard(window->getKeys(), deltaTime);

			glfwSetInputMode(window->getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			camera->processMouseMovement(window->getXChange(), window->getYChange(), true);
		}
		else
		{
			glfwSetInputMode(window->getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	

	// Update Uniform Buffers here
	FrameUniformBufferObject frameUboUpdate()
	{
		// Time calculation
		/*static auto lastTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();*/

		FrameUniformBufferObject ubo{};

		/*glm::mat4 zUpToYUpCorrection = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 modelBaseMatrix = zUpToYUpCorrection;

		ubo.model = glm::rotate(modelBaseMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/

		ubo.view = camera->calculateViewMatrix();
		ubo.proj = camera->getProjectionMatrix();
		ubo.proj[1][1] *= -1; // Vulkan's Y-coord is inverted in clip space

		return ubo;
	}

	void loadAssetsAndCreateRenderables()
	{
		//SceneObjectDefinition MetalBall{};
		//MetalBall.name = "Metal_PBR_Preview";
		//MetalBall.meshPath = "";
		//MetalBall.materialName = "Metal055A_4K";
		//MetalBall.albedoPath = "textures/Metal055A_4K/Metal055A_4K-PNG_Color.png";
		//MetalBall.normalPath = "textures/Metal055A_4K/Metal055A_4K-PNG_NormalDX.png";
		//MetalBall.ormPath = "textures/Metal055A_4K/Metal055A_4K-MRA.png";
		//MetalBall.displacementPath = "textures/Metal055A_4K/Metal055A_4K-PNG_Displacement.png";
		//MetalBall.position = glm::vec3(0.0, 0.0, 0.0);
		//MetalBall.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//MetalBall.useOrm = true;

		//SceneObjectDefinition TileFloor;
		//TileFloor.name = "TileFloor";
		//TileFloor.meshPath = "";
		//TileFloor.materialName = "Tiles107_2K";
		//TileFloor.albedoPath = "textures/Tiles107_2K/Tiles107_2K-PNG_Color.png";
		//TileFloor.normalPath = "textures/Tiles107_2K/Tiles107_2K-PNG_NormalDX.png";
		//TileFloor.ormPath = "textures/Tiles107_2K/Tiles107_2K-PNG_ORM.png";
		//TileFloor.displacementPath = "textures/Tiles107_2K/Tiles107_2K-PNG_Displacement.png";
		//TileFloor.position = glm::vec3(0.0, -2.5, 0.0);
		//TileFloor.defaultModel = PrimitiveModelType::CREATE_PLANE;
		//TileFloor.useOrm = true;

		//SceneObjectDefinition TileBall;
		//TileBall.name = "TileBall";
		//TileBall.meshPath = "";
		//TileBall.materialName = "Tiles107_2K";
		//TileBall.albedoPath = "textures/Tiles107_2K/Tiles107_2K-PNG_Color.png";
		//TileBall.normalPath = "textures/Tiles107_2K/Tiles107_2K-PNG_NormalDX.png";
		//TileBall.ormPath = "textures/Tiles107_2K/Tiles107_2K-PNG_ORM.png";
		//TileBall.displacementPath = "textures/Tiles107_2K/Tiles107_2K-PNG_Displacement.png";
		//TileBall.position = glm::vec3(30.0, 0.0, 0.0);
		//TileBall.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//TileBall.useOrm = true;

		//SceneObjectDefinition MetalBall2{};
		//MetalBall2.name = "Metal2_PBR_Preview";
		//MetalBall2.meshPath = "";
		//MetalBall2.materialName = "Metal049A_2K";
		//MetalBall2.albedoPath = "textures/Metal049A_2K/Metal049A_2K-PNG_Color.png";
		//MetalBall2.normalPath = "textures/Metal049A_2K/Metal049A_2K-PNG_NormalDX.png";
		//MetalBall2.ormPath = "textures/Metal049A_2K/Metal049A_2K-PNG_ORM.png";
		//MetalBall2.displacementPath = "textures/Metal049A_2K/Metal049A_2K-PNG_Displacement.png";
		//MetalBall2.position = glm::vec3(20.0, 0.0, 0.0);
		//MetalBall2.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//MetalBall2.useOrm = true;

		//SceneObjectDefinition GoldTileBall{};
		//GoldTileBall.name = "GoldTile_PBR_Preview";
		//GoldTileBall.meshPath = "";
		//GoldTileBall.materialName = "MetalTiles03_4K";
		//GoldTileBall.albedoPath = "textures/MetalTiles03_packed_4K/MetalTiles03_4K_BaseColor.png";
		//GoldTileBall.normalPath = "textures/MetalTiles03_packed_4K/MetalTiles03_4K_Normal.png";
		//GoldTileBall.ormPath = "textures/MetalTiles03_packed_4K/MetalTiles03_4K_ORM.png";
		//GoldTileBall.displacementPath = "textures/MetalTiles03_packed_4K/MetalTiles03_4K_Height.png";
		//GoldTileBall.position = glm::vec3(10.0, 0.0, 0.0);
		//GoldTileBall.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//GoldTileBall.useOrm = true;

		//SceneObjectDefinition MarbleBall{};
		//MarbleBall.name = "MarbleBall_PBR_Preview";
		//MarbleBall.meshPath = "";
		//MarbleBall.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//MarbleBall.materialName = "Marble04_2K";
		//MarbleBall.albedoPath = "textures/Marble04_2K/Marble04_2K_BaseColor.png";
		//MarbleBall.normalPath = "textures/Marble04_2K/Marble04_2K_Normal.png";
		//MarbleBall.roughnessPath = "textures/Marble04_2K/Marble04_2K_Roughness.png";
		////MarbleBall.displacementPath = "textures/Marble04_2K/Marble04_2K_Height.png";
		//MarbleBall.useOrm = false;
		//MarbleBall.position = glm::vec3(-10.0, 0.0, 0.0);

		//SceneObjectDefinition GroundBall{};
		//GroundBall.name = "GroundBall_PBR_Preview";
		//GroundBall.meshPath = "";
		//GroundBall.defaultModel = PrimitiveModelType::CREATE_SPHERE;
		//GroundBall.materialName = "Ground078_4K";
		//GroundBall.albedoPath = "textures/Ground078_4K/Ground078_4K-PNG_Color.png";
		//GroundBall.normalPath = "textures/Ground078_4K/Ground078_4K-PNG_NormalDX.png";
		//GroundBall.aoPath = "textures/Ground078_4K/Ground078_4K-PNG_AmbientOcclusion.png";
		//GroundBall.roughnessPath = "textures/Ground078_4K/Ground078_4K-PNG_Roughness.png";
		//GroundBall.displacementPath = "textures/Ground078_4K/Ground078_4K-PNG_Displacement.png";
		//GroundBall.useOrm = false;
		//GroundBall.position = glm::vec3(0.0, 0.0, 0.0);

		SceneObjectDefinition damagedHelmet{};
		damagedHelmet.meshFileType = MeshFileType::FILE_GLTF;
		damagedHelmet.name = "damagedHelmet";
		damagedHelmet.meshPath = "models/gltf/DamagedHelmet/DamagedHelmet.gltf";
		damagedHelmet.rotationAngles = glm::vec3(0.0f, 0.0f, 0.0f);

		SceneObjectDefinition metallicBall{};
		metallicBall.meshFileType = MeshFileType::FILE_GLTF;
		metallicBall.name = "metallicBall";
		metallicBall.meshPath = "models/gltf/CompareMetallic.glb";
		metallicBall.rotationAngles = glm::vec3(0.0f, 0.0f, 0.0f);
		metallicBall.position = glm::vec3(-10.0f, 0.0f, 0.0f);

		SceneObjectDefinition emissiveBall{};
		emissiveBall.meshFileType = MeshFileType::FILE_GLTF;
		emissiveBall.name = "metallicBall";
		emissiveBall.meshPath = "models/gltf/CompareEmissiveStrength.glb";
		emissiveBall.rotationAngles = glm::vec3(0.0f, 0.0f, 0.0f);
		emissiveBall.position = glm::vec3(-5.0f, 0.0f, 0.0f);

		SceneObjectDefinition roughnessBall{};
		roughnessBall.meshFileType = MeshFileType::FILE_GLTF;
		roughnessBall.name = "metallicBall";
		roughnessBall.meshPath = "models/gltf/CompareRoughness.glb";
		roughnessBall.rotationAngles = glm::vec3(0.0f, 0.0f, 0.0f);
		roughnessBall.position = glm::vec3(-15.0f, 0.0f, 0.0f);
		
		SceneObjectDefinition fruitBasket{};
		fruitBasket.meshFileType = MeshFileType::FILE_GLTF;
		fruitBasket.name = "fruitBasket";
		fruitBasket.meshPath = "models/gltf/CompareAmbientOcclusion/CompareAmbientOcclusion.gltf";
		fruitBasket.position = glm::vec3(10.0f, 0.0f, 0.0f);
		fruitBasket.scale = glm::vec3(10.0f);


		std::vector<SceneObjectDefinition> sceneDefinitions =
		{
			damagedHelmet,
			metallicBall,
			emissiveBall,
			roughnessBall,
			fruitBasket
		};
		for (const auto& def : sceneDefinitions)
		{
			if (def.meshFileType == MeshFileType::FILE_GLTF)
			{
				auto gltfRenderables = m_AssetManager->createRenderableObjectsFromGltf(def);
				renderableObjects.insert(renderableObjects.end(), gltfRenderables.begin(), gltfRenderables.end());
			}
		}
	}

	void updateObjectUniforms(uint32_t currentFrameIndex)
	{
		if (!objectDataDUBManager || renderableObjects.empty())
		{
			return;
		}
		for (uint32_t i = 0; i < renderableObjects.size(); ++i)
		{
			const auto& renderable = renderableObjects[i];
			ObjectUniformBufferObject objectUbo{};
			objectUbo.model = renderable.modelMatrix;
			objectDataDUBManager->updateDynamic(currentFrameIndex, i, objectUbo);
		}
	}

	void generateSkyboxCubeMap(VulkanTexture& hdrSourceTexture, VulkanTexture& destinationCubemap, uint32_t cubemapSize)
	{
		auto conversionRenderPass = std::make_unique<VulkanRenderPass>();
		conversionRenderPass->offscreen_rendering_create(devices->getLogicalDevice(), devices->getPhysicalDevice());
	
		auto conversionLayout = std::make_unique<VulkanDescriptorSetLayout>();
		conversionLayout->createForCubmapConversion(devices->getLogicalDevice());

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);

		auto conversionPipelineLayout = std::make_unique<VulkanPipelineLayout>();
		conversionPipelineLayout->create(devices->getLogicalDevice(), conversionLayout->getVkDescriptorSetLayout(), 1, &pushConstantRange);

		// descriptor set for 2D HDR texture that is being sampled NOW
		auto conversionDescriptorSet = std::make_unique<VulkanDescriptorSets>();
		conversionDescriptorSet->createForCubeMapConversion(devices->getLogicalDevice(), descriptorPool->getVkDescriptorPool(), conversionLayout->getVkDescriptorSetLayout(), hdrSourceTexture);
		
		auto conversionPipeline = std::make_unique<VulkanGraphicsPipeline>();
		conversionPipeline->createForConversion(
			devices->getLogicalDevice(),
			conversionPipelineLayout->getVkPipelineLayout(),
			conversionRenderPass->getVkRenderPass(),
			"shaders/equidirect_to_cube.vert.spv",
			"shaders/equidirect_to_cube.frag.spv"
		);

		std::vector<VkFramebuffer> framebuffers(6);
		std::vector<VkImageView> faceViews(6);

		for (uint32_t i = 0; i < 6; ++i) {
			// Create a temporary image view for a SINGLE face of the cubemap
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = destinationCubemap.getImage(); // The image handle from the cubemap object
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i; // This selects the face
			viewInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(devices->getLogicalDevice(), &viewInfo, nullptr, &faceViews[i]);

			VkFramebufferCreateInfo fbInfo{};
			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = conversionRenderPass->getVkRenderPass();
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = &faceViews[i];
			fbInfo.width = cubemapSize;
			fbInfo.height = cubemapSize;
			fbInfo.layers = 1;

			vkCreateFramebuffer(devices->getLogicalDevice(), &fbInfo, nullptr, &framebuffers[i]);

		}

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		captureProjection[1][1] *= -1;

		glm::mat4 captureViews[] =
		{
			// Right (+X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Left (-X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),

			// This is the view for LOOKING DOWN, which we store in the TOP (+Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),

			// This is the view for LOOKING UP, which we store in the BOTTOM (-Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),

			// Back (+Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Front (-Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
	
		VkCommandBuffer cmd = VulkanCommandBuffers::beginSingleTimeCommands(devices->getLogicalDevice(), commandPool->getVkCommandPool());

		for (uint32_t i = 0; i < 6; i++)
		{
			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // Define a clear color

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = conversionRenderPass->getVkRenderPass();
			renderPassBeginInfo.framebuffer = framebuffers[i];
			renderPassBeginInfo.renderArea.extent.width = cubemapSize;
			renderPassBeginInfo.renderArea.extent.height = cubemapSize;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = { 0.0f, 0.0f, (float)cubemapSize, (float)cubemapSize, 0.0f, 1.0f };
			VkRect2D scissor = { {0, 0}, {cubemapSize, cubemapSize} };
			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &scissor);


			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, conversionPipeline->getVkPipeline());

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
				conversionPipelineLayout->getVkPipelineLayout(),
				0, 1,
				conversionDescriptorSet->getVkDescriptorSetsRaw(),
				0, nullptr
			);
			
			glm::mat4 mvp = captureProjection * captureViews[i];
			vkCmdPushConstants(cmd, conversionPipelineLayout->getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

			VkBuffer vertexBuffers[] = { m_skyboxCubeVertexBuffer->getVkBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
			// draw 36 vertices of cube
			vkCmdDraw(cmd, 36, 1, 0, 0);

			vkCmdEndRenderPass(cmd);

		}

		VulkanCommandBuffers::endSingleTimeCommands(cmd, devices->getLogicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool());
		vkQueueWaitIdle(devices->getGraphicsQueue());

		VulkanImage::transitionImageLayout(
			devices->getLogicalDevice(),
			devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(),
			destinationCubemap.getImage(),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6 // <-- This is the layer count for the whole cubemap
		);

		for (uint32_t i = 0; i < 6; i++)
		{
			vkDestroyFramebuffer(devices->getLogicalDevice(), framebuffers[i], nullptr);
			vkDestroyImageView(devices->getLogicalDevice(), faceViews[i], nullptr);
		}

		conversionDescriptorSet->destroy();
		conversionDescriptorSet.reset();

		conversionPipeline->destroy();
		conversionPipeline.reset();

		conversionPipelineLayout->destroy();
		conversionPipelineLayout.reset();

		conversionLayout->destroy();
		conversionLayout.reset();

		conversionRenderPass->destroy();
		conversionRenderPass.reset();

	}

	// creates low-resolution, blurry version of skybox
	void generateIrradianceMap()
	{
		std::cout << "Generating Irradiance Map..." << std::endl;
		const uint32_t irradianceMapSize = 32;
		irradianceMap = std::make_unique<VulkanTexture>();
		irradianceMap->createCubemap(devices->getLogicalDevice(), devices->getPhysicalDevice(), irradianceMapSize, irradianceMapSize, 1);

		auto conversionRenderPass = std::make_unique<VulkanRenderPass>();
		conversionRenderPass->offscreen_rendering_create(devices->getLogicalDevice(), devices->getPhysicalDevice());

		auto irradianceLayout = std::make_unique<VulkanDescriptorSetLayout>();
		irradianceLayout->createForSkybox(devices->getLogicalDevice()); // Re-use skybox layout (UBO + samplerCube)

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);

		auto irradiancePipelineLayout = std::make_unique<VulkanPipelineLayout>();
		irradiancePipelineLayout->create(devices->getLogicalDevice(), irradianceLayout->getVkDescriptorSetLayout(), 1u, &pushConstantRange);

		auto irradianceConvDescriptorSet = std::make_unique<VulkanDescriptorSets>();
		irradianceConvDescriptorSet->createForSkybox(
			devices->getLogicalDevice(),
			descriptorPool->getVkDescriptorPool(),
			irradianceLayout->getVkDescriptorSetLayout(),
			1, // Only need one set for this process
			frameUboManager->getBuffers(), // This is a bit of a hack, we only need the layout
			*skyboxTexture
		);

		auto irradiancePipeline = std::make_unique<VulkanGraphicsPipeline>();
		irradiancePipeline->createForConversion(
			devices->getLogicalDevice(),
			irradiancePipelineLayout->getVkPipelineLayout(),
			conversionRenderPass->getVkRenderPass(),
			"shaders/equidirect_to_cube.vert.spv", // We can reuse the same vertex shader
			"shaders/irradiance.frag.spv"
		);

		std::vector<VkFramebuffer> framebuffers(6);
		std::vector<VkImageView> faceViews(6);

		for (uint32_t i = 0; i < 6; ++i) {
			// Create a temporary image view for a SINGLE face of the cubemap
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = irradianceMap->getImage(); // The image handle from the cubemap object
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i; // This selects the face
			viewInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(devices->getLogicalDevice(), &viewInfo, nullptr, &faceViews[i]);

			VkFramebufferCreateInfo fbInfo{};
			fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbInfo.renderPass = conversionRenderPass->getVkRenderPass();
			fbInfo.attachmentCount = 1;
			fbInfo.pAttachments = &faceViews[i];
			fbInfo.width = irradianceMapSize;
			fbInfo.height = irradianceMapSize;
			fbInfo.layers = 1;

			vkCreateFramebuffer(devices->getLogicalDevice(), &fbInfo, nullptr, &framebuffers[i]);

		}

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		captureProjection[1][1] *= -1;

		glm::mat4 captureViews[] =
		{
			// Right (+X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Left (-X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),

			// This is the view for LOOKING DOWN, which we store in the TOP (+Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),

			// This is the view for LOOKING UP, which we store in the BOTTOM (-Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),

			// Back (+Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Front (-Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		VkCommandBuffer cmd = VulkanCommandBuffers::beginSingleTimeCommands(devices->getLogicalDevice(), commandPool->getVkCommandPool());

		for (uint32_t i = 0; i < 6; i++)
		{
			VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // Define a clear color

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = conversionRenderPass->getVkRenderPass();
			renderPassBeginInfo.framebuffer = framebuffers[i];
			renderPassBeginInfo.renderArea.extent.width = irradianceMapSize;
			renderPassBeginInfo.renderArea.extent.height = irradianceMapSize;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = { 0.0f, 0.0f, (float)irradianceMapSize, (float)irradianceMapSize, 0.0f, 1.0f };
			VkRect2D scissor = { {0, 0}, {irradianceMapSize, irradianceMapSize} };
			vkCmdSetViewport(cmd, 0, 1, &viewport);
			vkCmdSetScissor(cmd, 0, 1, &scissor);


			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, irradiancePipeline->getVkPipeline());

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
				irradiancePipelineLayout->getVkPipelineLayout(),
				0, 1,
				irradianceConvDescriptorSet->getVkDescriptorSetsRaw(),
				0, nullptr
			);

			glm::mat4 mvp = captureProjection * captureViews[i];
			vkCmdPushConstants(cmd, irradiancePipelineLayout->getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);

			VkBuffer vertexBuffers[] = { m_skyboxCubeVertexBuffer->getVkBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
			vkCmdDraw(cmd, 36, 1, 0, 0);

			vkCmdEndRenderPass(cmd);
		}

		VulkanCommandBuffers::endSingleTimeCommands(cmd, devices->getLogicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool());
		vkQueueWaitIdle(devices->getGraphicsQueue());

		VulkanImage::transitionImageLayout(
			devices->getLogicalDevice(),
			devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(),
			irradianceMap->getImage(),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6
		);

		for (uint32_t i = 0; i < 6; i++)
		{
			vkDestroyFramebuffer(devices->getLogicalDevice(), framebuffers[i], nullptr);
			vkDestroyImageView(devices->getLogicalDevice(), faceViews[i], nullptr);
		}

		irradianceConvDescriptorSet->destroy();
		irradianceConvDescriptorSet.reset();

		irradiancePipeline->destroy();
		irradiancePipeline.reset();
		
		irradiancePipelineLayout->destroy();
		irradiancePipelineLayout.reset();

		irradianceLayout->destroy();
		irradianceLayout.reset();

		conversionRenderPass->destroy();
		conversionRenderPass.reset();
	}

	void generatePrefilerMap()
	{
		std::cout << "Generating Prefilter Map..." << std::endl;
		const uint32_t prefilterMapSize = 128;
		const uint32_t maxMipLevels = static_cast<uint32_t>(floor(log2(prefilterMapSize))) + 1;

		prefilterMap = std::make_unique<VulkanTexture>();
		prefilterMap->createCubemap(devices->getLogicalDevice(), devices->getPhysicalDevice(), prefilterMapSize, prefilterMapSize, maxMipLevels);

		auto conversionRenderPass = std::make_unique<VulkanRenderPass>();
		conversionRenderPass->offscreen_rendering_create(devices->getLogicalDevice(), devices->getPhysicalDevice());

		auto prefilerDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>();
		prefilerDescriptorSetLayout->createForSkybox(devices->getLogicalDevice()); // Re-use skybox layout (UBO + samplerCube)

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);

		VkPushConstantRange prefilterPushConstantRange{};
		prefilterPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		prefilterPushConstantRange.offset = sizeof(glm::mat4);
		prefilterPushConstantRange.size = sizeof(float);

		std::array<VkPushConstantRange, 2> pushConstants = { pushConstantRange, prefilterPushConstantRange };

		auto prefilterPipelineLayout = std::make_unique<VulkanPipelineLayout>();
		prefilterPipelineLayout->create(devices->getLogicalDevice(), prefilerDescriptorSetLayout->getVkDescriptorSetLayout(), 2u, pushConstants.data());

		auto prefilterDescriptorSet = std::make_unique<VulkanDescriptorSets>();
		prefilterDescriptorSet->createForSkybox(
			devices->getLogicalDevice(),
			descriptorPool->getVkDescriptorPool(),
			prefilerDescriptorSetLayout->getVkDescriptorSetLayout(),
			1, // Only need one set for this process
			frameUboManager->getBuffers(), // This is a bit of a hack, we only need the layout
			*skyboxTexture
		);

		auto prefilterPipeline = std::make_unique<VulkanGraphicsPipeline>();
		prefilterPipeline->createForConversion(
			devices->getLogicalDevice(),
			prefilterPipelineLayout->getVkPipelineLayout(),
			conversionRenderPass->getVkRenderPass(),
			"shaders/equidirect_to_cube.vert.spv",
			"shaders/prefilter.frag.spv" // NEW shader
		);


		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		captureProjection[1][1] *= -1;
		glm::mat4 captureViews[] =
		{
			// Right (+X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Left (-X)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),

			// This is the view for LOOKING DOWN, which we store in the TOP (+Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),

			// This is the view for LOOKING UP, which we store in the BOTTOM (-Y) face of the cubemap.
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),

			// Back (+Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			// Front (-Z)
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};


		for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
		{
			uint32_t mipWidth = static_cast<uint32_t>(prefilterMapSize * pow(0.5, mip));
			uint32_t mipHeight = static_cast<uint32_t>(prefilterMapSize * pow(0.5, mip));

			// You'll need to create a new framebuffer and image view for EACH mip level inside this loop
			// ... or create all mip-level image views at the start

			std::vector<VkFramebuffer> framebuffers(6);
			std::vector<VkImageView> faceViews(6);

			for (uint32_t i = 0; i < 6; ++i) {
				// Create a temporary image view for a SINGLE face of the cubemap
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = prefilterMap->getImage(); // The image handle from the cubemap object
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = mip;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = i; // This selects the face
				viewInfo.subresourceRange.layerCount = 1;

				vkCreateImageView(devices->getLogicalDevice(), &viewInfo, nullptr, &faceViews[i]);

				VkFramebufferCreateInfo fbInfo{};
				fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				fbInfo.renderPass = conversionRenderPass->getVkRenderPass();
				fbInfo.attachmentCount = 1;
				fbInfo.pAttachments = &faceViews[i];
				fbInfo.width = mipWidth;
				fbInfo.height = mipHeight;
				fbInfo.layers = 1;

				vkCreateFramebuffer(devices->getLogicalDevice(), &fbInfo, nullptr, &framebuffers[i]);
			}

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			// You'll need to pass this roughness value to the shader, likely via Push Constants.
			// Update your prefilter pipeline layout to accept a push constant.

			// Then, inside this mip loop, loop through the 6 faces and render, just like before.
			// Remember to set the viewport to mipWidth and mipHeight!

			VkCommandBuffer cmd = VulkanCommandBuffers::beginSingleTimeCommands(devices->getLogicalDevice(), commandPool->getVkCommandPool());


			for (uint32_t i = 0; i < 6; i++)
			{
				VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

				VkRenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = conversionRenderPass->getVkRenderPass();
				renderPassBeginInfo.framebuffer = framebuffers[i];
				renderPassBeginInfo.renderArea.extent.width = mipWidth;
				renderPassBeginInfo.renderArea.extent.height = mipWidth;
				renderPassBeginInfo.renderArea.offset = { 0, 0 };
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport = { 0.0f, 0.0f, (float)mipWidth, (float)mipHeight, 0.0f, 1.0f };
				VkRect2D scissor = { {0, 0}, {mipWidth, mipHeight} };
				vkCmdSetViewport(cmd, 0, 1, &viewport);
				vkCmdSetScissor(cmd, 0, 1, &scissor);


				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, prefilterPipeline->getVkPipeline());

				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					prefilterPipelineLayout->getVkPipelineLayout(),
					0, 1,
					prefilterDescriptorSet->getVkDescriptorSetsRaw(),
					0, nullptr
				);

				glm::mat4 mvp = captureProjection * captureViews[i];
				vkCmdPushConstants(cmd, prefilterPipelineLayout->getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), &mvp);
				vkCmdPushConstants(cmd, prefilterPipelineLayout->getVkPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(mvp), sizeof(roughness), &roughness);

				VkBuffer vertexBuffers[] = { m_skyboxCubeVertexBuffer->getVkBuffer() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
				// draw 36 vertices of cube
				vkCmdDraw(cmd, 36, 1, 0, 0);

				vkCmdEndRenderPass(cmd);
			}
			VulkanCommandBuffers::endSingleTimeCommands(cmd, devices->getLogicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool());
			vkQueueWaitIdle(devices->getGraphicsQueue());

			for (uint32_t i = 0; i < 6; i++)
			{
				vkDestroyFramebuffer(devices->getLogicalDevice(), framebuffers[i], nullptr);
				vkDestroyImageView(devices->getLogicalDevice(), faceViews[i], nullptr);
			}
		}

		VulkanImage::transitionImageLayout(
			devices->getLogicalDevice(),
			devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(),
			prefilterMap->getImage(),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			0,
			maxMipLevels
		);

		
		prefilterDescriptorSet->destroy();
		prefilterDescriptorSet.reset();

		prefilterPipeline->destroy();
		prefilterPipeline.reset();

		prefilterPipelineLayout->destroy();
		prefilterPipelineLayout.reset();

		prefilerDescriptorSetLayout->destroy();
		prefilerDescriptorSetLayout.reset();

		conversionRenderPass->destroy();
		conversionRenderPass.reset();
	}

	void generateBrdfLut()
	{
		std::cout << "Generating BRDF Lookup table..." << std::endl;
		const uint32_t lutSize = 512;
		brdfLut = std::make_unique<VulkanTexture>();
		brdfLut->createRenderableTexture(
			devices->getLogicalDevice(),
			devices->getPhysicalDevice(),
			lutSize, lutSize,
			VK_FORMAT_R16G16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		auto conversionRenderPass = std::make_unique<VulkanRenderPass>();
		conversionRenderPass->offscreen_rendering_create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VK_FORMAT_R16G16_SFLOAT);

		VkImageView brdfImageView = brdfLut->getImageView();
		VkFramebuffer lutFramebuffer;
		VkFramebufferCreateInfo fbInfo{};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = conversionRenderPass->getVkRenderPass();
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = &brdfImageView;
		fbInfo.width = lutSize;
		fbInfo.height = lutSize;
		fbInfo.layers = 1;
		vkCreateFramebuffer(devices->getLogicalDevice(), &fbInfo, nullptr, &lutFramebuffer);

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 0;
		layoutInfo.pBindings = nullptr;
		VkDescriptorSetLayout nullDescriptorSetLayout;
		vkCreateDescriptorSetLayout(devices->getLogicalDevice(), &layoutInfo, nullptr, &nullDescriptorSetLayout);

		auto brdfPipelineLayout = std::make_unique<VulkanPipelineLayout>();
		brdfPipelineLayout->create(devices->getLogicalDevice(), nullDescriptorSetLayout);

		auto brdfPipeline = std::make_unique<VulkanGraphicsPipeline>();
		brdfPipeline->createForLutGeneration(
			devices->getLogicalDevice(),
			brdfPipelineLayout->getVkPipelineLayout(),
			conversionRenderPass->getVkRenderPass(),
			"shaders/brdf.vert.spv", // A simple passthrough/fullscreen triangle shader
			"shaders/brdf.frag.spv"
		);


		// 4. Record commands to render a single fullscreen quad
		VkCommandBuffer cmd = VulkanCommandBuffers::beginSingleTimeCommands(devices->getLogicalDevice(), commandPool->getVkCommandPool());

		// Transition layout to be a render target
		VulkanImage::transitionImageLayout(devices->getLogicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), brdfLut->getImage(), VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// Begin render pass, bind pipeline, draw 1 triangle (3 vertices), end render pass
		
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = conversionRenderPass->getVkRenderPass();
		renderPassBeginInfo.framebuffer = lutFramebuffer;
		renderPassBeginInfo.renderArea.extent.width = lutSize;
		renderPassBeginInfo.renderArea.extent.height = lutSize;
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = { 0.0f, 0.0f, (float)lutSize, (float)lutSize, 0.0f, 1.0f };
		VkRect2D scissor = { {0, 0}, {lutSize, lutSize} };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, brdfPipeline->getVkPipeline());

		vkCmdDraw(cmd, 3, 1, 0, 0); // Draw 1 triangle (3 vertices)
		vkCmdEndRenderPass(cmd);

		VulkanCommandBuffers::endSingleTimeCommands(cmd, devices->getLogicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool());
		vkQueueWaitIdle(devices->getGraphicsQueue());

		// Transition layout to be a shader resource for sampling
		VulkanImage::transitionImageLayout(
			devices->getLogicalDevice(),
			devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(),
			brdfLut->getImage(),
			VK_FORMAT_R16G16_SFLOAT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);

		// 5. Cleanup
		vkDestroyFramebuffer(devices->getLogicalDevice(), lutFramebuffer, nullptr);

		vkDestroyDescriptorSetLayout(devices->getLogicalDevice(), nullDescriptorSetLayout, nullptr);

		brdfPipeline->destroy();
		brdfPipeline.reset();

		brdfPipelineLayout->destroy();
		brdfPipelineLayout.reset();

		conversionRenderPass->destroy();
		conversionRenderPass.reset();
	}

	void loadCubeModel() {
		// A cube has 6 faces, each with 2 triangles, for a total of 36 vertices.
		// We define a simple unit cube from -1 to 1 in each dimension.
		// The vertex positions are also the direction vectors we'll use for sampling the cubemap.
		const std::vector<glm::vec3> cubeVertices = {
			// positions          
			// Back face
			{-1.0f, -1.0f, -1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{-1.0f, -1.0f, -1.0f},
			{-1.0f,  1.0f, -1.0f},
			// Front face
			{-1.0f, -1.0f,  1.0f},
			{ 1.0f, -1.0f,  1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{-1.0f,  1.0f,  1.0f},
			{-1.0f, -1.0f,  1.0f},
			// Left face
			{-1.0f,  1.0f,  1.0f},
			{-1.0f,  1.0f, -1.0f},
			{-1.0f, -1.0f, -1.0f},
			{-1.0f, -1.0f, -1.0f},
			{-1.0f, -1.0f,  1.0f},
			{-1.0f,  1.0f,  1.0f},
			// Right face
			{ 1.0f,  1.0f,  1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{ 1.0f, -1.0f,  1.0f},
			// Bottom face
			{-1.0f, -1.0f, -1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{ 1.0f, -1.0f,  1.0f},
			{ 1.0f, -1.0f,  1.0f},
			{-1.0f, -1.0f,  1.0f},
			{-1.0f, -1.0f, -1.0f},
			// Top face
			{-1.0f,  1.0f, -1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{-1.0f,  1.0f, -1.0f},
			{-1.0f,  1.0f,  1.0f}
		};

		m_skyboxCubeVertexBuffer = std::make_unique<VulkanVertexBuffer>();
		// Note: Your VulkanVertexBuffer::create function likely takes a vector of `Vertex` structs.
		// You may need to adapt this call or create an overload that takes `glm::vec3`.
		// For simplicity, I'll assume you can adapt it.

		// A temporary conversion if your create function needs the full Vertex struct
		std::vector<Vertex> verticesForBuffer;
		verticesForBuffer.reserve(cubeVertices.size());
		for (const auto& pos : cubeVertices) {
			verticesForBuffer.push_back({ pos, {}, {}, {} }); // Only position is needed
		}

		m_skyboxCubeVertexBuffer->create(
			devices->getLogicalDevice(),
			devices->getPhysicalDevice(),
			devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(),
			verticesForBuffer
		);
	}

};

int main()
{
	VulkanEngine app;

	try 
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}