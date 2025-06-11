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

#include "Camera.h"
#include "Renderable.h"
#include "Material.h"
#include "Lights.h"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

//const int MAX_FRAMES_IN_FLIGHT = 2;

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

	// GLFW Window
	std::unique_ptr<Window> window;

	// VkInstance and DebugMessenger
	std::unique_ptr<VulkanInstance> instance;

	std::unique_ptr<VulkanSurface> surface;

	std::unique_ptr<VulkanDevice> devices;

	std::unique_ptr<VulkanSwapChain> swapChainObj;

	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout;
	std::unique_ptr<VulkanDescriptorPool> descriptorPool;
	std::unique_ptr<VulkanDescriptorSets> descriptorSets;
	std::unique_ptr<VulkanPipelineLayout> pipelineLayout;

	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelineFill;
	std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipelineWireframe;
	bool m_WireframeMode = false;

	std::unique_ptr<VulkanFramebuffers> swapChainFramebuffers;

	std::unique_ptr<VulkanCommandPool> commandPool;
	std::unique_ptr<VulkanCommandBuffers> commandBuffers;

	std::unique_ptr<VulkanSyncObjects> syncObjects;

	bool framebufferResized = false;

	std::unique_ptr<VulkanDepthResources> depthResourceObj;

	std::unique_ptr<VulkanRenderer> renderer;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::unique_ptr<Camera> camera;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	float deltaTime = 0.0f;
	float accumulatedTime = 0.0f;


	// --- Collection of Model resources --- 

	// mapped with mesh path string
	std::map<std::string, std::unique_ptr<VulkanVertexBuffer>> loadedVertexBuffers;
	std::map<std::string, std::unique_ptr<VulkanIndexBuffer>> loadedIndexBuffers;
	std::map<std::string, std::uint32_t> meshIndexCounts;

	// mapped with texture path string
	//std::map<std::string, std::unique_ptr<VulkanTexture>> loadedTextures;

	std::map<std::string, std::shared_ptr<Material>> loadedMaterials;

	std::vector<RenderableObject> renderableObjects;

	// UniformBuffers
	std::unique_ptr<VulkanUniformBuffers> frameUboManager;
	std::unique_ptr<VulkanUniformBuffers> objectDataDUBManager;
	std::unique_ptr<VulkanUniformBuffers> lightingUboManager;
	SceneLightingUBO sceneLights; // CPU SIDE DATA
	std::unique_ptr<VulkanUniformBuffers> tessellationUboManager;
	TessellationUBO tessUboData; // CPU SIDE DATA
	// -------------------------------------

	void initWindow()
	{

		const uint32_t WIDTH_CONST = 3840;
		const uint32_t HEIGHT_CONST = 2160;
		const std::string TITLE = "Vulkan";
		try
		{
			window = std::make_unique<Window>(WIDTH_CONST, HEIGHT_CONST, TITLE);
			/*window->setUserPointer(this);
			window->setFramebufferSizeCallback(framebufferResizeCallback);*/

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
		// VkInstance
		instance = std::make_unique<VulkanInstance>();
		instance->createInstance();
		instance->setupDebugMessenger(); // Debug
		// VkSurface
		surface = std::make_unique<VulkanSurface>();
		surface->createSurface(instance->getVkInstance(), window->getGlfwWindow());

		devices = std::make_unique<VulkanDevice>();
		devices->createDevices(instance->getVkInstance(), surface->getVkSurface(), deviceExtensions, validationLayers);

		swapChainObj = std::make_unique<VulkanSwapChain>();
		swapChainObj->create(devices->getPhysicalDevice(), devices->getLogicalDevice(), surface->getVkSurface(), window->getGlfwWindow());

		renderPass = std::make_unique<VulkanRenderPass>();
		renderPass->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), swapChainObj->getImageFormat());

		descriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>();
		descriptorSetLayout->create(devices->getLogicalDevice());

		pipelineLayout = std::make_unique<VulkanPipelineLayout>();
		pipelineLayout->create(devices->getLogicalDevice(), descriptorSetLayout->getVkDescriptorSetLayout());

		// --- Graphics Pipieline ---
		m_GraphicsPipelineFill = std::make_unique<VulkanGraphicsPipeline>();
		m_GraphicsPipelineFill->create(
			devices->getLogicalDevice(), 
			pipelineLayout->getVkPipelineLayout(),
			renderPass->getVkRenderPass(), 
			"shaders/tess.vert.spv", 
			"shaders/frag.spv",
			"shaders/tess.tesc.spv",
			"shaders/tess.tese.spv",
			VK_POLYGON_MODE_FILL
		);



		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(devices->getPhysicalDevice(), &deviceFeatures);
		
		if (deviceFeatures.fillModeNonSolid)
		{
			m_GraphicsPipelineWireframe = std::make_unique<VulkanGraphicsPipeline>();
			m_GraphicsPipelineWireframe->create(
				devices->getLogicalDevice(),
				pipelineLayout->getVkPipelineLayout(),
				renderPass->getVkRenderPass(),
				"shaders/tess.vert.spv",
				"shaders/wireframe.frag.spv",
				"shaders/tess.tesc.spv",
				"shaders/tess.tese.spv",
				VK_POLYGON_MODE_LINE // Specify line mode
			);
		}
		// ---------------------------

		commandPool = std::make_unique<VulkanCommandPool>();
		commandPool->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), surface->getVkSurface());

		depthResourceObj = std::make_unique<VulkanDepthResources>();
		depthResourceObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), swapChainObj->getExtent());

		swapChainFramebuffers = std::make_unique<VulkanFramebuffers>();
		swapChainFramebuffers->create(devices->getLogicalDevice(), swapChainObj->getImageViews(), depthResourceObj->getDepthImageView(), renderPass->getVkRenderPass(), swapChainObj->getExtent());

		loadAssetsAndCreateRenderables();

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
		sceneLights.dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 5.0f); //w intensity

		tessellationUboManager = std::make_unique<VulkanUniformBuffers>();
		tessellationUboManager->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, sizeof(TessellationUBO));

		// ------------------------

		descriptorPool = std::make_unique<VulkanDescriptorPool>();
		descriptorPool->create(devices->getLogicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(renderableObjects.size()));

		descriptorSets = std::make_unique<VulkanDescriptorSets>();
		descriptorSets->createForMaterials(
			devices->getLogicalDevice(),
			descriptorPool->getVkDescriptorPool(),
			descriptorSetLayout->getVkDescriptorSetLayout(),
			VulkanGlobals::MAX_FRAMES_IN_FLIGHT,
			frameUboManager->getBuffers(),
			objectDataDUBManager->getBuffers(),
			lightingUboManager->getBuffers(),
			tessellationUboManager->getBuffers(),
			loadedMaterials
		);

		commandBuffers = std::make_unique<VulkanCommandBuffers>();
		commandBuffers->create(devices->getLogicalDevice(), commandPool->getVkCommandPool(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT);

		syncObjects = std::make_unique<VulkanSyncObjects>();
		syncObjects->create(devices->getLogicalDevice(), VulkanGlobals::MAX_FRAMES_IN_FLIGHT, static_cast<uint32_t>(swapChainObj->getImageViews().size()));

		renderer = std::make_unique<VulkanRenderer>(
			*devices,            // Pass by reference
			*swapChainObj,
			*renderPass,
			*pipelineLayout,
			*m_GraphicsPipelineFill,
			*swapChainFramebuffers,
			*commandBuffers,
			*frameUboManager,
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
			50.0f                              // farF
		);
	}

	void mainLoop()
	{
		startTime = std::chrono::high_resolution_clock::now();
		accumulatedTime = 0.0f;

		bool m_M_KeyPressed = false;
		bool up_KeyPressed = false;
		bool down_KeyPressed = false;
		size_t tessLevelIndex = 0;
		std::array<float, 4> tessLevelValue{4.0f, 8.0f, 16.0f, 64.0f};

		while (window && !window->shouldClose())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = currentTime;
			accumulatedTime += deltaTime;

			window->pollEvents();

			camera->processKeyboard(window->getKeys(), deltaTime);
			camera->processMouseMovement(window->getXChange(), window->getYChange(), true);

			 /*if (window->getKeys()[GLFW_KEY_UP] && !up_KeyPressed)
			 { 
				 tessLevelIndex = std::min(tessLevelIndex + 1, tessLevelValue.size() - 1);
				 tessUboData.tessellationLevel = tessLevelValue[tessLevelIndex];
				 up_KeyPressed = true;
			 }
			 if (!window->getKeys()[GLFW_KEY_UP])
			 {
				 up_KeyPressed = false;
			 }
			 if (window->getKeys()[GLFW_KEY_DOWN] && !down_KeyPressed)
			 { 
				 tessLevelIndex = std::max(static_cast<size_t>(0), tessLevelIndex - 1);
				 tessUboData.tessellationLevel = tessLevelValue[tessLevelIndex];
				 down_KeyPressed = true;
			 }
			 if (!window->getKeys()[GLFW_KEY_DOWN])
			 {
				 down_KeyPressed = false;
			 }*/

			 // Safely check key states with bounds validation
			const auto& keys = window->getKeys();

			if (keys[GLFW_KEY_UP] && !up_KeyPressed)
			{
				if (tessLevelIndex + 1 < tessLevelValue.size())
				{
					++tessLevelIndex;
					try
					{
						tessUboData.tessellationLevel = tessLevelValue.at(tessLevelIndex);
						// Optional: Log for debugging
						// std::cout << "Tessellation Level: " << tessUboData.tessellationLevel << std::endl;
					}
					catch (const std::out_of_range& e)
					{
						std::cerr << "Tessellation array access error: " << e.what() << std::endl;
						tessLevelIndex = 0; // Reset to safe index
						tessUboData.tessellationLevel = tessLevelValue.at(0);
					}
					up_KeyPressed = true;
				}
			}
			if (!keys[GLFW_KEY_UP])
			{
				up_KeyPressed = false;
			}

			if (keys[GLFW_KEY_DOWN] && !down_KeyPressed)
			{
				if (tessLevelIndex > 0)
				{
					--tessLevelIndex;
					try
					{
						tessUboData.tessellationLevel = tessLevelValue.at(tessLevelIndex);
						// Optional: Log for debugging
						// std::cout << "Tessellation Level: " << tessUboData.tessellationLevel << std::endl;
					}
					catch (const std::out_of_range& e)
					{
						std::cerr << "Tessellation array access error: " << e.what() << std::endl;
						tessLevelIndex = 0; // Reset to safe index
						tessUboData.tessellationLevel = tessLevelValue.at(0);
					}
					down_KeyPressed = true;
				}
			}
			if (!keys[GLFW_KEY_DOWN])
			{
				down_KeyPressed = false;
			}

			 if (window->getKeys()[GLFW_KEY_M] && !m_M_KeyPressed)
			 {
				 if (m_GraphicsPipelineWireframe)
				 {

					 m_WireframeMode = !m_WireframeMode;
					 //printf("Wireframe mode toggled: %b\n", m_WireframeMode);

				 }
				 m_M_KeyPressed = true;
			 }
			 if (!window->getKeys()[GLFW_KEY_M]) 
			 {
				 m_M_KeyPressed = false;
			 }


			uint32_t uboFrameIndex = renderer->getCurrentFrame();

			tessellationUboManager->update(uboFrameIndex, tessUboData);

			sceneLights.viewPosition = glm::vec4(camera->getCameraPosition(), 1.0f);
			//lightingUboManager->updateLights(uboFrameIndex, sceneLights);
			lightingUboManager->update(uboFrameIndex, sceneLights);

			updateObjectUniforms(uboFrameIndex);

			VkPipeline pipelineToUse = m_WireframeMode
				? m_GraphicsPipelineWireframe->getVkPipeline()
				: m_GraphicsPipelineFill->getVkPipeline();

			renderer->drawFrame(
				pipelineToUse,
				[this]() { return this->frameUboUpdate(); },
				framebufferResized,
				[this]() { return this->recreateSwapChain(); },
				renderableObjects,
				objectDataDUBManager->getDynamicAlignment()
			);
		}

		if (devices->isInitialized())
		{
			vkDeviceWaitIdle(devices->getLogicalDevice());
		}
	}

	void cleanupSwapChain()
	{

		depthResourceObj->destroy();

		swapChainFramebuffers->destroy();

		swapChainObj->destroy();
	}

	void cleanup()
	{
		if (camera) camera.reset();

		cleanupSwapChain();

		if (renderer) renderer.reset();

		for (auto& pair : loadedVertexBuffers)
		{
			if (pair.second) pair.second->destroy();
		}
		loadedVertexBuffers.clear();

		for (auto& pair : loadedIndexBuffers)
		{
			if (pair.second) pair.second->destroy();
		}
		loadedIndexBuffers.clear();

		/*for (auto& pair : loadedTextures)
		{
			if (pair.second) pair.second->destroy();
		}
		loadedTextures.clear();*/

		for (auto& pair : loadedMaterials)
		{
			if (pair.second)
			{
				pair.second->albedoMap->destroy();
				pair.second->albedoMap.reset();
				pair.second->normalMap->destroy();
				pair.second->normalMap.reset();
				pair.second->ormMap->destroy();
				pair.second->ormMap.reset();
				pair.second->displacementMap->destroy();
				pair.second->displacementMap.reset();
			}
		}
		loadedMaterials.clear();

		if (frameUboManager) frameUboManager->destroy();
		frameUboManager.reset();

		if (objectDataDUBManager) objectDataDUBManager->destroy();
		objectDataDUBManager.reset();

		if (lightingUboManager) lightingUboManager->destroy();
		lightingUboManager.reset();

		if (tessellationUboManager) tessellationUboManager->destroy();
		tessellationUboManager.reset();

		if (syncObjects) syncObjects->destroy();
		syncObjects.reset();

		if (descriptorSets) descriptorSets->destroy();
		descriptorSets.reset();

		if (descriptorPool) descriptorPool->destroy();
		descriptorPool.reset();

		if (m_GraphicsPipelineFill) m_GraphicsPipelineFill->destroy();
		m_GraphicsPipelineFill.reset();

		if (m_GraphicsPipelineWireframe) m_GraphicsPipelineWireframe->destroy();
		m_GraphicsPipelineWireframe.reset();

		if (pipelineLayout) pipelineLayout->destroy();
		pipelineLayout.reset();

		if (descriptorSetLayout) descriptorSetLayout->destroy();
		descriptorSetLayout.reset();

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

	void recreateSwapChain()
	{
		window->waitForRestoredSize();

		vkDeviceWaitIdle(devices->getLogicalDevice());

		cleanupSwapChain();

		swapChainObj->create(devices->getPhysicalDevice(), devices->getLogicalDevice(), surface->getVkSurface(), window->getGlfwWindow());
		depthResourceObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), swapChainObj->getExtent());
		swapChainFramebuffers->create(devices->getLogicalDevice(), swapChainObj->getImageViews(), depthResourceObj->getDepthImageView(), renderPass->getVkRenderPass(), swapChainObj->getExtent());
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
		std::vector<SceneObjectDefinition> sceneDefinitions =
		{
			/*{"viking_room_1", MODEL_PATH, TEXTURE_PATH, glm::vec3(0.0f, 0.0f, 0.0f)},
			{"viking_room_2", MODEL_PATH, TEXTURE_PATH, glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 45.0f)}*/
			{
				"Metal_PBR_Preview", 
				"", 
				"Metal055A_4K", 
				"textures/Metal055A_4K/Metal055A_4K-PNG_Color.png", // Albedo
				"textures/Metal055A_4K/Metal055A_4K-PNG_NormalDX.png", //Normal
				"textures/Metal055A_4K/Metal055A_4K-MRA.png", // ORM
				"textures/Metal055A_4K/Metal055A_4K-PNG_Displacement.png", //Displacement
				glm::vec3(0.0, 0.0, 0.0)
			},
			{
				"Rock_PBR_Preview",
				"",
				"Rock061_4K",
				"textures/Rock061_4K/Rock061_4K-PNG_Color.png",
				"textures/Rock061_4K/Rock061_4K-PNG_NormalDX.png",
				"textures/Rock061_4K/Rock061_4K-PNG_ORM.png",
				"textures/Rock061_4K/Rock061_4K-PNG_Displacement.png",
				glm::vec3(10.0, 0.0, 0.0)
			}
		};

		for (const auto& def : sceneDefinitions)
		{
			if (loadedVertexBuffers.find(def.meshPath) == loadedVertexBuffers.end()) // If mesh is not loaded
			{
				// --- Load/Get Mesh ---
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;

				if (def.meshPath.size() == 0)
				{
					ModelLoader::createSphere(2.5, 16, 16, vertices, indices);
				}
				else
				{
					ModelLoader::loadModel(def.meshPath, vertices, indices);
				}
				

				auto vb = std::make_unique<VulkanVertexBuffer>();
				vb->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), vertices);
				loadedVertexBuffers[def.meshPath] = std::move(vb);
				 
				auto ib = std::make_unique<VulkanIndexBuffer>();
				ib->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), indices);
				loadedIndexBuffers[def.meshPath] = std::move(ib);
				meshIndexCounts[def.meshPath] = static_cast<uint32_t>(indices.size());
			}

			if (loadedMaterials.find(def.materalName) == loadedMaterials.end())
			{
				// --- Load/Get Material ---
				auto material = std::make_shared<Material>();
				auto albedo = std::make_shared<VulkanTexture>();
				albedo->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), def.albedoPath);
				
				auto normal = std::make_shared<VulkanTexture>();
				if (def.normalPath.size() == 0)
				{
					normal->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), "textures/default_normal.png");
				}
				else
				{
					normal->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), def.normalPath);
				}
				
				auto orm = std::make_shared<VulkanTexture>();
				if (def.ormPath.size() == 0)
				{
					orm->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), "textures/default_orm.png");
				}
				else
				{
					orm->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), def.ormPath);
				}

				auto displacement = std::make_shared<VulkanTexture>();
				if (def.displacementPath.size() == 0)
				{
					displacement->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), "textures/default_orm.png");
				}
				else
				{
					displacement->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), def.displacementPath);
				}
				

				material->name = def.materalName;
				material->albedoMap = albedo;
				material->normalMap = normal;
				material->ormMap = orm;
				material->displacementMap = displacement;

				loadedMaterials[def.materalName] = material;
			}

			// --- Create RenderableObject ---
			RenderableObject renderable;
			renderable.vertexBuffer = loadedVertexBuffers[def.meshPath].get();
			renderable.indexBuffer = loadedIndexBuffers[def.meshPath].get();
			renderable.indexCount = meshIndexCounts[def.meshPath];
			renderable.material = loadedMaterials[def.materalName];

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, def.position);
			model = glm::rotate(model, glm::radians(def.rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f)); // z axis
			model = glm::rotate(model, glm::radians(def.rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f)); // y axis
			model = glm::rotate(model, glm::radians(def.rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f)); // y axis
			model = glm::scale(model, def.scale);
			renderable.modelMatrix = model;

			renderableObjects.push_back(renderable);
		}
	}

	void updateObjectUniforms(uint32_t currentFrameIndex)
	{
		if (!objectDataDUBManager || renderableObjects.empty())
		{
			return;
		}

		// Optional: Z-up to Y-up correction if your models are Z-up
		// and you want to apply it uniformly before individual object transforms.
		glm::mat4 coordinateSystemCorrection = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		for (uint32_t i = 0; i < renderableObjects.size(); ++i)
		{
			const auto& renderable = renderableObjects[i];
			ObjectUniformBufferObject objectUbo{};
			// The modelMatrix in RenderableObject should be its complete world transform
			// If you applied a global rotation (like the spinning viking room) previously
			// in the FrameUniformBufferObject, that logic should now be applied here
			// to each renderableObject's modelMatrix if desired.

			// Example: If renderable.modelMatrix is just the static placement
			// and you want to add a dynamic rotation like before:
			// float time = accumulatedTime; // Or pass time specifically
			// glm::mat4 dynamicRotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			// objectUbo.model = dynamicRotation * renderable.modelMatrix;


			// Assuming renderable.modelMatrix is what you want to render
			//objectUbo.model = renderable.modelMatrix;

			// If coordinateSystemCorrection is needed and not already baked into renderabel.modelMatrix:
			objectUbo.model = coordinateSystemCorrection * renderable.modelMatrix;

			objectDataDUBManager->updateDynamic(currentFrameIndex, i, objectUbo);
		}
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