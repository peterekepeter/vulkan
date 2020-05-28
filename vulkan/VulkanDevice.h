#pragma once
#include "./VulkanShaderModule.h"
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
	VulkanShaderModule CreateShaderModule(const std::vector<char> binary);
	VulkanShaderModule CreateShaderModule(const char* binary, const size_t size);

	~VulkanDevice();
};