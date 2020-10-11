#pragma once
#include <vulkan/vulkan_core.h>
#include <stdexcept>

class VulkanCommandPool
{
public:
	VkCommandPool m_vk_command_pool;
	VkDevice m_vk_device;

	VulkanCommandPool(VkDevice device, VkCommandPoolCreateInfo& info)
		:m_vk_device(device)
	{
		if (vkCreateCommandPool(m_vk_device, &info, nullptr, &m_vk_command_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	~VulkanCommandPool() {
		vkDestroyCommandPool(m_vk_device, m_vk_command_pool, nullptr);
	}

	class Builder
	{
	public:
		VkDevice m_vk_device;
		uint32_t m_queue_family_index = -1;
		bool m_transient = false;
		bool m_individual_reset = false;

		Builder(VkDevice device) : m_vk_device(device) {}
		operator VulkanCommandPool() { return build(); }

		Builder& queue_family_index(uint32_t i) { m_queue_family_index = i; return *this; }
		Builder& transient_buffers() { m_transient = true; return *this; }
		Builder& reset_buffers() { m_individual_reset = true; return *this; }

		VulkanCommandPool build() {
			VkCommandPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.queueFamilyIndex = m_queue_family_index;
			info.flags = 0;
			if (m_transient) {
				info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			}
			if (m_individual_reset) {
				info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			}
			return VulkanCommandPool(m_vk_device, info);
		}
	};
};