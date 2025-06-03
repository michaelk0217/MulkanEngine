#include "VulkanSurface.h"

VulkanSurface::VulkanSurface() : instance(VK_NULL_HANDLE), surface(VK_NULL_HANDLE)
{
}

VulkanSurface::~VulkanSurface()
{
	if (surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

}

VkSurfaceKHR VulkanSurface::getVkSurface() const
{
	if (surface == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Vulkan Surface get call before creation!");
	}
	return surface;
}

void VulkanSurface::createSurface(VkInstance vkinstance, GLFWwindow* window)
{
	instance = vkinstance;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}