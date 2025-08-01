#include "VulkanDevice.h"

VulkanDevice::VulkanDevice() : device(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE), instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE), graphicsQueue(VK_NULL_HANDLE), presentQueue(VK_NULL_HANDLE)
{

}

VulkanDevice::~VulkanDevice()
{
	vkDestroyDevice(device, nullptr);
}

bool VulkanDevice::isInitialized() const
{
	return device != VK_NULL_HANDLE && physicalDevice != VK_NULL_HANDLE;
}

void VulkanDevice::createDevices(VkInstance vkinsatnce, VkSurfaceKHR vksurface, const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers)
{
	instance = vkinsatnce;
	surface = vksurface;

	deviceExtensionsTmp = deviceExtensions;
	validationLayersTmp = validationLayers;

	pickPhysicalDevice(instance);
	createLogicalDevice();
}




VkDevice VulkanDevice::getLogicalDevice() const
{
	if (device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vulkan Device get called before creation!");
	}
	return device;
}

VkPhysicalDevice VulkanDevice::getPhysicalDevice() const
{
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vulkan PhysicalDevice get called before creation!");
	}
	return physicalDevice;
}

VkQueue VulkanDevice::getGraphicsQueue() const
{
	if (graphicsQueue == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vulkan Graphics Queue get called before creation!");
	}
	return graphicsQueue;
}

VkQueue VulkanDevice::getPresentQueue() const
{
	if (presentQueue == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vulkan Present Queue get called before creation!");
	}
	return presentQueue;
}

void VulkanDevice::pickPhysicalDevice(VkInstance instance)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& physdevice : devices)
	{
		if (isDeviceSuitable(physdevice))
		{
			physicalDevice = physdevice;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void VulkanDevice::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };


	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.tessellationShader = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsTmp.size());
	createInfo.ppEnabledExtensionNames = deviceExtensionsTmp.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersTmp.size());
		createInfo.ppEnabledLayerNames = validationLayersTmp.data();
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice physdevice)
{

	QueueFamilyIndices indices = findQueueFamilies(physdevice, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(physdevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physdevice, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physdevice, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && supportedFeatures.fillModeNonSolid;
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice physdevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physdevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physdevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensionsTmp.begin(), deviceExtensionsTmp.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkphysdevice, VkSurfaceKHR vksurfacekhr)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkphysdevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkphysdevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkphysdevice, i, vksurfacekhr, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}
