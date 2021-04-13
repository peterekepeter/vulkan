#pragma once

class VulkanGraphicsPipeline
{
	DECLARE_MOVEABLE_TYPE(VulkanGraphicsPipeline)
public:
	VkPipeline m_vk_pipeline;
	VkDevice m_vk_device;

	VulkanGraphicsPipeline(VkDevice, const VkGraphicsPipelineCreateInfo&);
	VulkanGraphicsPipeline(VkDevice, VkPipelineCache, const VkGraphicsPipelineCreateInfo&);

private:
	void Init(VkPipelineCache, const VkGraphicsPipelineCreateInfo&);
};