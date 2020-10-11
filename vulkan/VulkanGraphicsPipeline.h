#pragma once

class VulkanGraphicsPipeline
{
public:
	VkPipeline m_vk_pipeline;
	VkDevice m_vk_device;

	VulkanGraphicsPipeline(VkDevice, const VkGraphicsPipelineCreateInfo&);
	VulkanGraphicsPipeline(VkDevice, VkPipelineCache, const VkGraphicsPipelineCreateInfo&);

	// no copy
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
	VulkanGraphicsPipeline& operator =(const VulkanGraphicsPipeline&) = delete;

	// movable
	VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept;
	VulkanGraphicsPipeline& operator =(VulkanGraphicsPipeline&& other) noexcept;

	~VulkanGraphicsPipeline();

private:
	void Init(VkPipelineCache, const VkGraphicsPipelineCreateInfo&);
	void Steal(VulkanGraphicsPipeline& other);
	void Free();
};