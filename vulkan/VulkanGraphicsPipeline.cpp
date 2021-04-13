#include "stdafx.h"
#include "./VulkanGraphicsPipeline.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
	VkDevice device, 
	const VkGraphicsPipelineCreateInfo& info)
	: m_vk_device(device)
	, m_vk_pipeline(VK_NULL_HANDLE)
{
	Init(VK_NULL_HANDLE, info);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
	VkDevice device, 
	VkPipelineCache cache, 
	const VkGraphicsPipelineCreateInfo& info)
	: m_vk_device(device)
	, m_vk_pipeline(VK_NULL_HANDLE)
{
	Init(cache, info);
}
 
void VulkanGraphicsPipeline::Init(
	VkPipelineCache cache,
	const VkGraphicsPipelineCreateInfo& info)
{
	auto result = vkCreateGraphicsPipelines(
		m_vk_device, cache, 1, &info, NULL, &this->m_vk_pipeline);
	ensure(result == VK_SUCCESS, "grapichs pipline was created");
}

void VulkanGraphicsPipeline::move_members(VulkanGraphicsPipeline&& other)
{
	m_vk_device = other.m_vk_device;
	m_vk_pipeline = other.m_vk_pipeline;
	other.m_vk_pipeline = VK_NULL_HANDLE;
}

void VulkanGraphicsPipeline::free_members()
{
	if (m_vk_pipeline == VK_NULL_HANDLE) {
		return;
	}
	vkDestroyPipeline(m_vk_device, m_vk_pipeline, nullptr);
	m_vk_pipeline = VK_NULL_HANDLE;
}
