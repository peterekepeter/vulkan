#pragma once
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include "VulkanCommandBuffer.h"

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

	VulkanCommandBuffer allocate_command_buffer() {
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandPool = m_vk_command_pool;
		info.commandBufferCount = 1;
		VulkanCommandBuffer result;

		if (vkAllocateCommandBuffers(m_vk_device, nullptr, &result.m_vk_command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}
		return result;
	}

	void free_command_buffer(const VulkanCommandBuffer& buffer) {
		vkFreeCommandBuffers(m_vk_device, m_vk_command_pool, 1, &buffer.m_vk_command_buffer);
	}

	std::vector<VulkanCommandBuffer> allocate_command_buffers(uint32_t count) {
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandPool = m_vk_command_pool;
		info.commandBufferCount = count;

		std::vector<VkCommandBuffer> buffers;
		buffers.resize(count);

		if (vkAllocateCommandBuffers(m_vk_device, &info, buffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		std::vector<VulkanCommandBuffer> result;
		result.resize(count);
		for (int i = 0; i < count; i++) {
			result[i].m_vk_command_buffer = buffers[i];
		}

		return result;
	}

	void free_command_buffers(const std::vector<VulkanCommandBuffer>& command_buffers)
	{
		auto count = command_buffers.size();
		std::vector<VkCommandBuffer> buffers;
		buffers.resize(count);
		for (int i = 0; i < count; i++) {
			buffers[i] = command_buffers[i].m_vk_command_buffer;
		}
		vkFreeCommandBuffers(m_vk_device, m_vk_command_pool, buffers.size(), buffers.data());
	}

	void reset_command_buffer(VulkanCommandBuffer buffer) { reset_individual_buffer(buffer.m_vk_command_buffer, false); }
	void reset_command_buffer(VkCommandBuffer vk_buffer) { reset_individual_buffer(vk_buffer, false); }
	void reset_and_free_command_buffer(VulkanCommandBuffer buffer) { reset_individual_buffer(buffer.m_vk_command_buffer, true); }
	void reset_and_free_command_buffer(VkCommandBuffer vk_buffer) { reset_individual_buffer(vk_buffer, true); }

	void reset_command_pool() { reset_command_pool_impl(false); }
	void reset_and_free_command_pool() { reset_command_pool_impl(true); }
	

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
		Builder& transient() { m_transient = true; return *this; }
		Builder& individual_reset() { m_individual_reset = true; return *this; }

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

private:

	void reset_individual_buffer(VkCommandBuffer buffer, bool free) {
		auto flags = free ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0;
		if (vkResetCommandBuffer(buffer, flags) != VK_SUCCESS) {
			throw std::runtime_error("failed to reset command buffer!");
		}
	}

	void reset_command_pool_impl(bool free) {
		auto flags = free ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0;
		if (vkResetCommandPool(m_vk_device, m_vk_command_pool, flags) != VK_SUCCESS) {
			throw std::runtime_error("failed to reset command pool!");
		}
	}

};