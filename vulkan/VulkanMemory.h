#pragma once

class VulkanMemory
{
public:
	VkDevice m_vk_device;
	VkDeviceMemory m_vk_memory;

	VulkanMemory(VkDevice device, uint64_t size, VulkanPhysicalMemory::TypeIndex memory_type) : m_vk_device(device) {
		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = size;
		allocateInfo.memoryTypeIndex = memory_type.index;
		switch (vkAllocateMemory(device, &allocateInfo, nullptr, &m_vk_memory)) {
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

	VulkanMemory() : m_vk_device(VK_NULL_HANDLE), m_vk_memory(VK_NULL_HANDLE) { }

	VulkanMemory(VulkanMemory&& other) noexcept {
		m_vk_device = other.m_vk_device;
		m_vk_memory = other.m_vk_memory;
		other.m_vk_memory = VK_NULL_HANDLE;
	}

	VulkanMemory& operator =(VulkanMemory&& other) noexcept {
		if (m_vk_memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_vk_device, m_vk_memory, nullptr);
		}
		m_vk_device = other.m_vk_device;
		m_vk_memory = other.m_vk_memory;
		other.m_vk_memory = VK_NULL_HANDLE;
		return *this;
	}

	VulkanMemory(const VulkanMemory&) = delete;
	VulkanMemory& operator =(const VulkanMemory&) = delete;

	~VulkanMemory() {
		if (m_vk_memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_vk_device, m_vk_memory, nullptr);
			m_vk_memory = VK_NULL_HANDLE;
		}
	}
};
