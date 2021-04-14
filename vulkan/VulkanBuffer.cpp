#include "pch.h"
#include "VulkanBuffer.h"


VulkanBuffer::VulkanBuffer()
	: device(nullptr)
	, physicalDevice(nullptr) {}

VulkanBuffer::VulkanBuffer(
	VkDevice device,
	VkPhysicalDevice physical,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties)
	: device(device)
	, physicalDevice(physical)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t VulkanBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBuffer::free_members()
{
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (bufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, bufferMemory, nullptr);
		bufferMemory = VK_NULL_HANDLE;
	}
}

void VulkanBuffer::move_members(VulkanBuffer&& other)
{
	device = other.device;
	physicalDevice = other.physicalDevice;
	buffer = other.buffer;
	bufferMemory = other.bufferMemory;
	other.buffer = VK_NULL_HANDLE;
	other.bufferMemory = VK_NULL_HANDLE;
}
