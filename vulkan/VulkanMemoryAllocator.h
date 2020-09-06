#pragma once
#include "VulkanMemory.h"

class VulkanMemoryAllocator
{
public:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VulkanPhysicalMemory phisicalMemory;

	VulkanMemoryAllocator(VkPhysicalDevice phyiscalDevice, VkDevice device)
		: physicalDevice(phyiscalDevice), device(device), phisicalMemory(phyiscalDevice) { }

	VulkanMemory Allocate(uint64_t size, VulkanPhysicalMemory::TypeIndex memoryTypeIndex) {
		return VulkanMemory(device, size, memoryTypeIndex);
	}
};
