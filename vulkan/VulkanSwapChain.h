#pragma once
#include "Vulkan.hpp"
#include "VulkanDevice.h"

class VulkanSwapChain {
public:
	VkSwapchainKHR swapChain;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	VkDevice logicalDevice;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VulkanSwapChain(const VulkanPhysicalDevice& physicalDevice, const VulkanDevice& device, int defaultWidth, int defaultHeight);
	~VulkanSwapChain();
};