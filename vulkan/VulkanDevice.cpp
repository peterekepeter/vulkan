#include "pch.h"
#include "./VulkanDevice.h"

VulkanDevice::VulkanDevice(VulkanApplication& vulkan, VulkanPhysicalDevice& physicalDevice)
{
	// setup queue create info
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = {
		physicalDevice.graphicsFamilyIndex,
		physicalDevice.computeFamilyIndex,
		physicalDevice.presentFamilyIndex
	};
	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// setup required features
	deviceFeatures = {};

	// setup require extensions
	requiredDeviceExtensions = physicalDevice.requiredDeviceExtensions;

	// setup logical device creation
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan.requiredLayers.size());
	createInfo.ppEnabledLayerNames = vulkan.requiredLayers.data();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.enabledExtensionCount = requiredDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

	if (vkCreateDevice(physicalDevice.physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(device, physicalDevice.graphicsFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, physicalDevice.computeFamilyIndex, 0, &computeQueue);
	vkGetDeviceQueue(device, physicalDevice.presentFamilyIndex, 0, &presentQueue);
}

VulkanShaderModule VulkanDevice::CreateShaderModule(const std::vector<char> binary)
{
	return VulkanShaderModule(device, binary);
}

VulkanShaderModule VulkanDevice::CreateShaderModule(const char* binary, const size_t size)
{
	return VulkanShaderModule(device, binary, size);
}

VulkanDevice::~VulkanDevice() {
	vkDestroyDevice(device, nullptr);
}
