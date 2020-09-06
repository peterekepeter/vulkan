#pragma once

class VulkanPhysicalMemory
{
public:
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceMemoryProperties memProperties;

	VulkanPhysicalMemory(VkPhysicalDevice physicalDevice) : physicalDevice(physicalDevice) {
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	}

	// typesafe wrapper for memory type index
	class TypeIndex {
	public:
		uint32_t index;
	};

	// helpers
	TypeIndex FindHostVisibleAndCoherentMemoryTypeIndex(const VkMemoryRequirements& requirements) { return FindHostVisibleAndCoherentMemoryTypeIndex(requirements.memoryTypeBits); }
	TypeIndex FindDeviceLocalMemoryTypeIndex(const VkMemoryRequirements& requirements) { return FindDeviceLocalMemoryTypeIndex(requirements.memoryTypeBits); }
	TypeIndex FindMemoryTypeIndex(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags requiredProperties = 0) { return FindMemoryTypeIndex(requirements.memoryTypeBits, requiredProperties); }
	TypeIndex FindHostVisibleAndCoherentMemoryTypeIndex(uint32_t memoryTypeBitsRequirement) { return FindMemoryTypeIndex(memoryTypeBitsRequirement, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); }
	TypeIndex FindDeviceLocalMemoryTypeIndex(uint32_t memoryTypeBitsRequirement) { return FindMemoryTypeIndex(memoryTypeBitsRequirement, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }

	TypeIndex FindMemoryTypeIndex(
		uint32_t memoryTypeBitsRequirement,
		VkMemoryPropertyFlags requiredProperties = 0)
	{
		const uint32_t memoryCount = memProperties.memoryTypeCount;
		for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
			const uint32_t memoryTypeBits = (1 << memoryIndex);
			const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

			const VkMemoryPropertyFlags properties =
				memProperties.memoryTypes[memoryIndex].propertyFlags;
			const bool hasRequiredProperties = requiredProperties == 0 ||
				(properties & requiredProperties) == requiredProperties;

			if (isRequiredMemoryType && hasRequiredProperties)
				return TypeIndex{ memoryIndex };
		}

		throw std::runtime_error("FindMemoryTypeIndex: failed to find requested memory type");
	}
};