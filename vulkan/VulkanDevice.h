#pragma once
#include "./VulkanShader.h"
#include "./Vulkan.hpp"

class VulkanDevice
{
public:
	VkDeviceQueueCreateInfo queueCreateInfo;
	VkPhysicalDeviceFeatures deviceFeatures;
	std::vector<const char*> requiredDeviceExtensions;
	VkDeviceCreateInfo createInfo;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue presentQueue;

	VulkanDevice(VulkanApplication& vulkan, VulkanPhysicalDevice& physicalDevice);
	VulkanShader CreateShader(const std::vector<char> binary);
	VulkanShader CreateShader(const char* binary, const size_t size);

	~VulkanDevice();
};