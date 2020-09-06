#pragma once
#include "VulkanPhysicalMemory.h";
#include "VulkanMemoryAllocator.h";

class VulkanMemoryRequirementsAllocator
{
public:
	VulkanPhysicalMemory physicalMemory;
	VulkanMemoryAllocator allocator;

	VulkanMemoryRequirementsAllocator(VkPhysicalDevice phyiscalDevice, VkDevice device)
		: allocator(phyiscalDevice, device), physicalMemory(phyiscalDevice) { }

	VulkanMemory AllocateDeviceLocal(const VkMemoryRequirements& requirements) {
		auto index = physicalMemory.FindDeviceLocalMemoryTypeIndex(requirements.memoryTypeBits);
		return allocator.Allocate(requirements.size, index);
	}

	VulkanMemory AllocateHostVisibleAndCoherent(const VkMemoryRequirements& requirements) {
		auto index = physicalMemory.FindHostVisibleAndCoherentMemoryTypeIndex(requirements.memoryTypeBits);
		return allocator.Allocate(requirements.size, index);
	}

	VulkanMemory Allocate(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags requiredProperties = 0) {
		auto index = physicalMemory.FindMemoryTypeIndex(requirements.memoryTypeBits, requiredProperties);
		return allocator.Allocate(requirements.size, index);
	}
};