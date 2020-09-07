#pragma once
#include <vulkan\vulkan_core.h>

namespace CommonImageFormats
{
	// these image formats are actually useful and also happen to
	// have 100% compatibility with proper vulkan implementations

	const VkFormat R8G8B8A8_UNORM = VK_FORMAT_R8G8B8A8_UNORM;
	const VkFormat R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB;
	const VkFormat R16G16B16A16_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	const VkFormat R32G32B32A32_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT;
	const VkFormat R8G8_UNORM = VK_FORMAT_R8G8_UNORM;
	const VkFormat R16G16_SFLOAT = VK_FORMAT_R16G16_SFLOAT;
	const VkFormat R32G32_SFLOAT = VK_FORMAT_R32G32_SFLOAT;
	const VkFormat R8_UNORM = VK_FORMAT_R8_UNORM;
	const VkFormat R16_SFLOAT = VK_FORMAT_R16_SFLOAT;
	const VkFormat R32_SFLOAT = VK_FORMAT_R32_SFLOAT;
}
