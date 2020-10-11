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
		m_vk_device, 
		cache, 
		1, 
		&info, 
		NULL, 
		&this->m_vk_pipeline);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VulkanGraphicsPipeline::Steal(VulkanGraphicsPipeline& other)
{
	m_vk_device = other.m_vk_device;
	m_vk_pipeline = other.m_vk_pipeline;
	other.m_vk_pipeline = VK_NULL_HANDLE;
}

void VulkanGraphicsPipeline::Free()
{
	if (m_vk_pipeline == VK_NULL_HANDLE) {
		return;
	}
	vkDestroyPipeline(m_vk_device, m_vk_pipeline, nullptr);
	m_vk_pipeline = VK_NULL_HANDLE;
}
