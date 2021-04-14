#pragma once

class VulkanBuffer
{
	DECLARE_MOVEABLE_TYPE(VulkanBuffer)
private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
public:
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;

	VulkanBuffer();
	VulkanBuffer(
		VkDevice device,
		VkPhysicalDevice physical,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};