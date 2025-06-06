#pragma once

#ifndef VULKAN_GLOBALS_H
#define VULKAN_GLOBALS_H

#include <stdint.h>

namespace VulkanGlobals
{
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	constexpr size_t MAX_EXPECTED_OBJECTS = 1000;
}

#endif // !VULKAN_GLOBALS_H
