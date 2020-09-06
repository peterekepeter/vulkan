#pragma once

class VulkanMemory
{
public:
	VkDevice vkDeviceHandle;
	VkDeviceMemory vkMemoryHandle;

	VulkanMemory(VkDevice device, uint64_t size, VulkanPhysicalMemory::TypeIndex memory_type) : vkDeviceHandle(device) {
		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = size;
		allocateInfo.memoryTypeIndex = memory_type.index;
		switch (vkAllocateMemory(device, &allocateInfo, nullptr, &vkMemoryHandle)) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkAllocateMemory: out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkAllocateMemory: out of device memory");
		default:
			throw std::runtime_error("vkAllocateMemory: failed");
		}
	}

	VulkanMemory() : vkDeviceHandle(VK_NULL_HANDLE), vkMemoryHandle(VK_NULL_HANDLE) { }

	VulkanMemory(VulkanMemory&& other) noexcept {
		vkDeviceHandle = other.vkDeviceHandle;
		vkMemoryHandle = other.vkMemoryHandle;
		other.vkMemoryHandle = VK_NULL_HANDLE;
	}

	VulkanMemory& operator =(VulkanMemory&& other) noexcept {
		if (vkMemoryHandle != VK_NULL_HANDLE)
		{
			vkFreeMemory(vkDeviceHandle, vkMemoryHandle, nullptr);
		}
		vkDeviceHandle = other.vkDeviceHandle;
		vkMemoryHandle = other.vkMemoryHandle;
		other.vkMemoryHandle = VK_NULL_HANDLE;
		return *this;
	}

	VulkanMemory(const VulkanMemory&) = delete;
	VulkanMemory& operator =(const VulkanMemory&) = delete;

	~VulkanMemory() {
		if (vkMemoryHandle != VK_NULL_HANDLE)
		{
			vkFreeMemory(vkDeviceHandle, vkMemoryHandle, nullptr);
			vkMemoryHandle = VK_NULL_HANDLE;
		}
	}
};
