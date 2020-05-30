#pragma once
#include "VulkanGraphicsPipeline.h"
#include <vector>


class VulkanGraphicsPipelineBuilder
{
public:

	VulkanGraphicsPipelineBuilder(VkDevice vkDevice) 
		: vkDevice(vkDevice) { }

	VulkanGraphicsPipeline Build();
	operator VulkanGraphicsPipeline();

	VulkanGraphicsPipelineBuilder& AddShaderStage(
		const VkPipelineShaderStageCreateInfo& stage);

	VulkanGraphicsPipelineBuilder& SetVertexInputState(
		const VkPipelineVertexInputStateCreateInfo& state)
	{
		
	}


	// no copy
	VulkanGraphicsPipelineBuilder(const VulkanGraphicsPipelineBuilder& other) = delete;
	VulkanGraphicsPipelineBuilder& operator =(const VulkanGraphicsPipelineBuilder& other) = delete;

	// no move
	VulkanGraphicsPipelineBuilder(VulkanGraphicsPipelineBuilder&& other) = delete;
	VulkanGraphicsPipelineBuilder& operator =(VulkanGraphicsPipelineBuilder&& other) = delete;


private:
	VkDevice vkDevice = nullptr;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
};