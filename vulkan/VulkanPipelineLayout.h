#pragma once
#include <vulkan\vulkan_core.h>
#include <vector>

class VulkanPipelineLayout
{
	DECLARE_MOVEABLE_TYPE(VulkanPipelineLayout);
public:
	VkDevice m_vk_device;
	VkPipelineLayout m_vk_pipeline_layout;

	VulkanPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo& info)
		: m_vk_device(device)
	{
		auto create_pipeline_layout_result = 
			vkCreatePipelineLayout(device, &info, nullptr, &m_vk_pipeline_layout);
		ensure(create_pipeline_layout_result == VK_SUCCESS);
	}

	class Builder 
	{
	public:
		VkDevice m_vk_device;
		std::vector<VkDescriptorSetLayout> m_layouts;

		Builder(VkDevice device)
			: m_vk_device(device)
		{
		}

		Builder& add(VkDescriptorSetLayout layout) { m_layouts.push_back(layout); return *this; }

		operator VulkanPipelineLayout() {
			VkPipelineLayoutCreateInfo m_info = {};
			m_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			m_info.setLayoutCount = m_layouts.size();
			m_info.pSetLayouts = m_layouts.data();
			return VulkanPipelineLayout(m_vk_device, m_info);
		}
	};

};

void VulkanPipelineLayout::move_members(VulkanPipelineLayout&& from)
{
	m_vk_device = from.m_vk_device;
	m_vk_pipeline_layout = from.m_vk_pipeline_layout;
	from.m_vk_pipeline_layout = VK_NULL_HANDLE;
}

void VulkanPipelineLayout::free_members()
{
	if (m_vk_pipeline_layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout, nullptr);
	}
}