#include "VulkanCommandPool.h"

VulkanCommandPool::VulkanCommandPool() : commandPool(VK_NULL_HANDLE), device(VK_NULL_HANDLE)
{
}

VulkanCommandPool::~VulkanCommandPool()
{
	destroy();
}

void VulkanCommandPool::create(VkDevice vkdevice, VkPhysicalDevice vkphysicaldevice, VkSurfaceKHR vksurface)
{
	device = vkdevice;
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vkphysicaldevice, vksurface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanCommandPool::destroy()
{
	if (commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(device, commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}
}

VkCommandPool VulkanCommandPool::getVkCommandPool() const
{
	if (commandPool == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Get command pool called before initialization!");
	}
	return commandPool;
}
