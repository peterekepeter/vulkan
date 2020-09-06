#pragma once
#include "VulkanMemoryRequirementsAllocator.h"


class VulkanImageMemoryAllocator
{
public:
	VulkanMemoryRequirementsAllocator allocator;
	VkDevice device;

	VulkanImageMemoryAllocator(VkPhysicalDevice phyiscalDevice, VkDevice device)
		: allocator(phyiscalDevice, device), device(device) { }

	VulkanMemory AllocateDeviceLocal(VkImage image) {
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device, image, &requirements);
		return allocator.AllocateDeviceLocal(requirements);
	}

	VulkanMemory AllocateHostVisibleAndCoherent(VkImage image) {
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device, image, &requirements);
		return allocator.AllocateHostVisibleAndCoherent(requirements);
	}

	VulkanMemory Allocate(VkImage image, VkMemoryPropertyFlags requiredProperties = 0) {
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(device, image, &requirements);
		return allocator.Allocate(requirements, requiredProperties);
	}

	void BindImageMemory(VkImage image, VulkanMemory& memory)
	{
		vkBindImageMemory(memory.vkDeviceHandle, image, memory.vkMemoryHandle, VkDeviceSize(0));
	}

};