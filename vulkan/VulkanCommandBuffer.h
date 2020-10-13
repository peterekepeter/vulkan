#pragma once
#include <vulkan/vulkan_core.h>

class VulkanCommandBuffer
{
public:
	VkCommandBuffer m_vk_command_buffer;

	VulkanCommandBuffer() : m_vk_command_buffer(VK_NULL_HANDLE) { }
	VulkanCommandBuffer(VkCommandBuffer buffer) : m_vk_command_buffer(buffer) { }

};