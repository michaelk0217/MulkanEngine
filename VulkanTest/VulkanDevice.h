#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <set>
#include <optional>
#include "VulkanSwapChain.h"
#include "VulkanInstance.h"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR vksurface);

class VulkanDevice
{
public:
	VulkanDevice();
	~VulkanDevice();

	void createDevices(VkInstance insatnce, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions,const std::vector<const char*>& validationLayers);

	bool isInitialized() const;

	VkDevice getLogicalDevice() const;
	VkPhysicalDevice getPhysicalDevice() const;
	VkQueue getGraphicsQueue() const;
	VkQueue getPresentQueue() const;


private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkInstance instance;
	VkSurfaceKHR surface;
	std::vector<const char*> deviceExtensionsTmp;
	std::vector<const char*> validationLayersTmp;

	void createLogicalDevice();
	void pickPhysicalDevice(VkInstance instance);
	bool isDeviceSuitable(VkPhysicalDevice physdevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice physdevice);


};

