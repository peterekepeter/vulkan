#pragma once
#include "VulkanDevice.h"
#include "VulkanShaderModule.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"
#include "VulkanDescriptorPool.h"
#include "VulkanCommandPool.h"
#include "VulkanFramebuffer.h"
#include "VulkanSampler.h"

// Does not map to any vulkan concept, this is just a high-level class that 
// makes it easier to create objects.
class VulkanObjectBuilder
{
public:
	VkDevice m_vk_device;

	VulkanObjectBuilder(VulkanDevice& dev) : m_vk_device(dev.device) { }

	VulkanShaderModule shader_module(const std::vector<char> bin) { return VulkanShaderModule(m_vk_device, bin); }
	VulkanShaderModule shader_module(const char* bin, const size_t sz) { return VulkanShaderModule(m_vk_device, bin, sz); }
	VulkanRenderPass::Builder render_pass() { return VulkanRenderPass::Builder(m_vk_device); }
	VulkanDescriptorSetLayout::Builder descriptor_set_layout() { return VulkanDescriptorSetLayout::Builder(m_vk_device); }
	VulkanPipelineLayout::Builder pipeline_layout() { return VulkanPipelineLayout::Builder(m_vk_device); }
	VulkanGraphicsPipelineBuilder graphics_pipeline() { return VulkanGraphicsPipelineBuilder(m_vk_device); }
	VulkanDescriptorPool::Builder descriptor_pool() { return VulkanDescriptorPool::Builder(m_vk_device); }
	VulkanCommandPool::Builder command_pool() { return VulkanCommandPool::Builder(m_vk_device); }
	VulkanFramebuffer::Builder framebuffer() { return VulkanFramebuffer::Builder(m_vk_device); }
	VulkanSampler::Builder sampler() { return VulkanSampler::Builder(m_vk_device); }
};