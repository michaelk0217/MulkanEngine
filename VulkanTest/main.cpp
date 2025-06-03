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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

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
	std::unique_ptr<VulkanGraphicsPipeline> graphicsPipeline;

	std::unique_ptr<VulkanFramebuffers> swapChainFramebuffers;

	std::unique_ptr<VulkanCommandPool> commandPool;
	std::unique_ptr<VulkanCommandBuffers> commandBuffers;

	std::unique_ptr<VulkanSyncObjects> syncObjects;

	bool framebufferResized = false;

	uint32_t currentFrame = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;


	std::unique_ptr<VulkanVertexBuffer> vertexBufferObj;
	std::unique_ptr<VulkanIndexBuffer> indexBufferObj;

	std::unique_ptr<VulkanUniformBuffers> uniformBuffersObj;

	std::unique_ptr<VulkanTexture> textureObj;

	std::unique_ptr<VulkanDepthResources> depthResourceObj;

	std::unique_ptr<VulkanRenderer> renderer;

	std::unique_ptr<Camera> camera;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	float deltaTime = 0.0f;
	float accumulatedTime = 0.0f;

	void initWindow()
	{

		const uint32_t WIDTH_CONST = 1920;
		const uint32_t HEIGHT_CONST = 1080;
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

		graphicsPipeline = std::make_unique<VulkanGraphicsPipeline>();
		graphicsPipeline->create(devices->getLogicalDevice(), pipelineLayout->getVkPipelineLayout(),
			renderPass->getVkRenderPass(), "shaders/vert.spv", "shaders/frag.spv");

		commandPool = std::make_unique<VulkanCommandPool>();
		commandPool->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), surface->getVkSurface());

		depthResourceObj = std::make_unique<VulkanDepthResources>();
		depthResourceObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(), commandPool->getVkCommandPool(), swapChainObj->getExtent());

		swapChainFramebuffers = std::make_unique<VulkanFramebuffers>();
		swapChainFramebuffers->create(devices->getLogicalDevice(), swapChainObj->getImageViews(), depthResourceObj->getDepthImageView(), renderPass->getVkRenderPass(), swapChainObj->getExtent());

		textureObj = std::make_unique<VulkanTexture>();
		textureObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), commandPool->getVkCommandPool(), devices->getGraphicsQueue(), TEXTURE_PATH);

		ModelLoader::loadModel(MODEL_PATH, vertices, indices);

		vertexBufferObj = std::make_unique<VulkanVertexBuffer>();
		vertexBufferObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(), vertices);

		indexBufferObj = std::make_unique<VulkanIndexBuffer>();
		indexBufferObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), devices->getGraphicsQueue(),
			commandPool->getVkCommandPool(), indices);

		uniformBuffersObj = std::make_unique<VulkanUniformBuffers>();
		uniformBuffersObj->create(devices->getLogicalDevice(), devices->getPhysicalDevice(), MAX_FRAMES_IN_FLIGHT);

		descriptorPool = std::make_unique<VulkanDescriptorPool>();
		descriptorPool->create(devices->getLogicalDevice(), MAX_FRAMES_IN_FLIGHT);
		descriptorSets = std::make_unique<VulkanDescriptorSets>();
		descriptorSets->create(devices->getLogicalDevice(), descriptorPool->getVkDescriptorPool(), descriptorSetLayout->getVkDescriptorSetLayout(), MAX_FRAMES_IN_FLIGHT, uniformBuffersObj->getBuffers(), textureObj->getImageView(), textureObj->getSampler());

		commandBuffers = std::make_unique<VulkanCommandBuffers>();
		commandBuffers->create(devices->getLogicalDevice(), commandPool->getVkCommandPool(), MAX_FRAMES_IN_FLIGHT);

		syncObjects = std::make_unique<VulkanSyncObjects>();
		syncObjects->create(devices->getLogicalDevice(), MAX_FRAMES_IN_FLIGHT);

		renderer = std::make_unique<VulkanRenderer>(
			*devices,            // Pass by reference
			*swapChainObj,
			*renderPass,
			*pipelineLayout,
			*graphicsPipeline,
			*swapChainFramebuffers,
			*commandBuffers,
			*descriptorSets,
			*vertexBufferObj,
			*indexBufferObj,
			*uniformBuffersObj,
			*syncObjects,
			indices,             // Pass the actual indices vector
			MAX_FRAMES_IN_FLIGHT
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
		while (window && !window->shouldClose())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = currentTime;
			accumulatedTime += deltaTime;

			window->pollEvents();

			camera->processKeyboard(window->getKeys(), deltaTime);
			camera->processMouseMovement(window->getXChange(), window->getYChange(), true);

			renderer->drawFrame(
				[this]() { return this->uboUpdate(); },
				framebufferResized,
				[this]() { return this->recreateSwapChain(); }
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

		if (textureObj) textureObj->destroy();
		textureObj.reset();

		if (uniformBuffersObj) uniformBuffersObj->destroy();
		uniformBuffersObj.reset();

		if (indexBufferObj) indexBufferObj->destroy();
		indexBufferObj.reset();

		if (vertexBufferObj) vertexBufferObj->destroy();
		vertexBufferObj.reset();

		if (syncObjects) syncObjects->destroy();
		syncObjects.reset();

		if (descriptorSets) descriptorSets->destroy();
		descriptorSets.reset();

		if (descriptorPool) descriptorPool->destroy();
		descriptorPool.reset();

		if (graphicsPipeline) graphicsPipeline->destroy();
		graphicsPipeline.reset();

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
	UniformBufferObject uboUpdate()
	{
		static auto lastTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();


		UniformBufferObject ubo{};

		glm::mat4 zUpToYUpCorrection = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 modelBaseMatrix = zUpToYUpCorrection;

		ubo.model = glm::rotate(modelBaseMatrix, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = camera->calculateViewMatrix();

		//ubo.proj = glm::perspective(glm::radians(45.0f), swapChainObj->getExtent().width / (float)swapChainObj->getExtent().height, 0.1f, 10.0f);
		ubo.proj = camera->getProjectionMatrix();
		ubo.proj[1][1] *= -1;

		return ubo;
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