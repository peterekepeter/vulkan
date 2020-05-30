#pragma once
#include "VulkanGraphicsPipeline.h"
#include <vector>


class VulkanGraphicsPipelineBuilder
{
public:

	VulkanGraphicsPipelineBuilder(VkDevice vkDevice);

	VulkanGraphicsPipeline Build();
	operator VulkanGraphicsPipeline();

	VulkanGraphicsPipelineBuilder& AddShaderStage(
		const VkPipelineShaderStageCreateInfo& stage);

	VulkanGraphicsPipelineBuilder& SetVertexInputState(
		const VkPipelineVertexInputStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetInputAssemblyState(
		const VkPipelineInputAssemblyStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetViewportState(
		VkPipelineViewportStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetRasterizerState(
		VkPipelineRasterizationStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetMultisampleState(
		const VkPipelineMultisampleStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetColorBlendState(
		const VkPipelineColorBlendStateCreateInfo& state);

	VulkanGraphicsPipelineBuilder& SetPipelineLayout(
		const VkPipelineLayout& pipelineLayout);

	VulkanGraphicsPipelineBuilder& SetRenderPass(
		const VkRenderPass& renderPass);

	VulkanGraphicsPipelineBuilder& SetSubpassIndex(
		uint32_t subpassIndex);


	// no copy
	VulkanGraphicsPipelineBuilder(const VulkanGraphicsPipelineBuilder& other) = default;
	VulkanGraphicsPipelineBuilder& operator =(const VulkanGraphicsPipelineBuilder& other) = default;

	// no move
	VulkanGraphicsPipelineBuilder(VulkanGraphicsPipelineBuilder&& other) = default;
	VulkanGraphicsPipelineBuilder& operator =(VulkanGraphicsPipelineBuilder&& other) = default;


private:
	VkDevice vkDevice = nullptr;
	
	// state
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizerState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;

	// renderpass
	VkPipelineLayout vkPipelineLayout;
	VkRenderPass vkRenderPass;
	uint32_t subpass;
};


inline VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(VkDevice vkDevice)
	: vkDevice(vkDevice)
{
	vertexInputState = {};
	inputAssemblyState = {};
	viewportState = {};
	rasterizerState = {};
	multisampleState = {};
	colorBlendState = {};
	subpass = 0;
	vkPipelineLayout = VK_NULL_HANDLE;
	vkRenderPass = VK_NULL_HANDLE;
}

inline VulkanGraphicsPipeline VulkanGraphicsPipelineBuilder::Build()
{
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.stageCount = shaderStages.size();
	info.pStages = shaderStages.data();
	info.pVertexInputState = &vertexInputState;
	info.pInputAssemblyState = &inputAssemblyState;
	info.pViewportState = &viewportState;
	info.pRasterizationState = &rasterizerState;
	info.pMultisampleState = &multisampleState;
	info.pDepthStencilState = nullptr; // Optional
	info.pColorBlendState = &colorBlendState;
	info.pDynamicState = nullptr; // Optional

	info.layout = vkPipelineLayout;
	info.renderPass = vkRenderPass;
	info.subpass = subpass;

	info.basePipelineHandle = VK_NULL_HANDLE; // Optional
	info.basePipelineIndex = -1; // Optional
	return VulkanGraphicsPipeline(vkDevice, info);
}

inline VulkanGraphicsPipelineBuilder::operator VulkanGraphicsPipeline()
{
	return Build();
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddShaderStage(const VkPipelineShaderStageCreateInfo& stage)
{
	shaderStages.push_back(stage); // will perform copy
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetVertexInputState(const VkPipelineVertexInputStateCreateInfo& state)
{
	vertexInputState = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state)
{
	inputAssemblyState = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetViewportState(VkPipelineViewportStateCreateInfo& state) {
	viewportState = state; // has pointers
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetRasterizerState(VkPipelineRasterizationStateCreateInfo& state)
{
	rasterizerState = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetMultisampleState(const VkPipelineMultisampleStateCreateInfo& state)
{
	multisampleState = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetColorBlendState(const VkPipelineColorBlendStateCreateInfo& state)
{
	colorBlendState = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetPipelineLayout(const VkPipelineLayout& pipelineLayout)
{
	vkPipelineLayout = pipelineLayout;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetRenderPass(const VkRenderPass& renderPass)
{
	vkRenderPass = renderPass;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetSubpassIndex(uint32_t subpassIndex)
{
	subpass = subpassIndex;
	return *this;
}
