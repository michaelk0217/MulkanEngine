#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

class VulkanSurface
{
public:
	VulkanSurface();
	~VulkanSurface();

	VulkanSurface(const VulkanSurface&) = delete;
	VulkanSurface& operator=(const VulkanSurface&) = delete;
	VulkanSurface(VulkanSurface&&) = delete;
	VulkanSurface& operator=(VulkanSurface&&) = delete;

	VkSurfaceKHR getVkSurface() const;

	void createSurface(VkInstance vkinstance, GLFWwindow* window);

private:
	VkInstance instance;
	VkSurfaceKHR surface;
};

