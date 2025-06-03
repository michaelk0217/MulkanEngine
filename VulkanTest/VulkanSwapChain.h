#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>
#include "VulkanDevice.h"
#include "VulkanImage.h"

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

class VulkanSwapChain
{
public:
	VulkanSwapChain();
	~VulkanSwapChain();

	VkSwapchainKHR getSwapChain() const;
	std::vector<VkImageView> getImageViews() const;
	VkExtent2D getExtent() const;
	VkFormat getImageFormat() const;

	void create(VkPhysicalDevice physdevice, VkDevice vkdevice, VkSurfaceKHR vksurfacekhr, GLFWwindow* win);
	void destroy();


private:
	// Key Members
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages{};
	std::vector<VkImageView> swapChainImageViews{};
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	// Auxillary Members
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceKHR surface;
	GLFWwindow* window;

	// Helper Methods
	void createSwapChain();
	void createImageViews();

	bool checkParamReq() const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

