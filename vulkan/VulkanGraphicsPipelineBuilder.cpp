#include "stdafx.h"
#include "./VulkanGraphicsPipelineBuilder.h"

VulkanGraphicsPipeline VulkanGraphicsPipelineBuilder::Build()
{
	return this->operator VulkanGraphicsPipeline;
}

VulkanGraphicsPipelineBuilder::operator VulkanGraphicsPipeline()
{
	VkGraphicsPipelineCreateInfo createInfo = {};
	return VulkanGraphicsPipeline(vkDevice, createInfo);
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddShaderStage(const VkPipelineShaderStageCreateInfo& stage)
{
	shaderStages.push_back(stage); // will perform copy
	return *this;
}
