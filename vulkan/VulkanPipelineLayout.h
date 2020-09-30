#pragma once
#include <vulkan\vulkan_core.h>
#include <vector>

class VulkanPipelineLayout
{
public:
	VkDevice m_vk_device;
	VkPipelineLayout m_vk_pipeline_layout;

	VulkanPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo& info)
		: m_vk_device(device)
	{
		vkCreatePipelineLayout(device, &info, nullptr, &m_vk_pipeline_layout);
	}

	~VulkanPipelineLayout() {
		vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout, nullptr);
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