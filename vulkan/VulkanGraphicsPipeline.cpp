#include "stdafx.h"
#include "./VulkanGraphicsPipeline.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
	VkDevice device, 
	const VkGraphicsPipelineCreateInfo& info)
	: vkDevice(device)
	, vkPipeline(VK_NULL_HANDLE)
{
	Init(VK_NULL_HANDLE, info);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
	VkDevice device, 
	VkPipelineCache cache, 
	const VkGraphicsPipelineCreateInfo& info)
	: vkDevice(device)
	, vkPipeline(VK_NULL_HANDLE)
{
	Init(cache, info);
}
 
VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept
{
	Steal(other);
}

VulkanGraphicsPipeline& VulkanGraphicsPipeline::operator=(VulkanGraphicsPipeline&& other) noexcept
{
	Free();
	Steal(other);
	return *this;
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	Free();
}

void VulkanGraphicsPipeline::Init(
	VkPipelineCache cache,
	const VkGraphicsPipelineCreateInfo& info)
{
	auto result = vkCreateGraphicsPipelines(
		vkDevice, 
		cache, 
		1, 
		&info, 
		NULL, 
		&this->vkPipeline);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VulkanGraphicsPipeline::Steal(VulkanGraphicsPipeline& other)
{
	vkDevice = other.vkDevice;
	vkPipeline = other.vkPipeline;
	other.vkPipeline = VK_NULL_HANDLE;
}

void VulkanGraphicsPipeline::Free()
{
	if (vkPipeline == VK_NULL_HANDLE) {
		return;
	}
	vkDestroyPipeline(vkDevice, vkPipeline, nullptr);
	vkPipeline = VK_NULL_HANDLE;
}
