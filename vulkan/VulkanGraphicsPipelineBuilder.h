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

	VulkanGraphicsPipelineBuilder& AddShaderStage(
		VkShaderStageFlagBits vlStageFlagBits,
		VkShaderModule vkShaderModule,
		const char* entryPointName = nullptr, // defaults to "main"
		VkSpecializationInfo* vkPSpecializationInfo = nullptr
	);

	VulkanGraphicsPipelineBuilder& AddVertexShaderStage(
		const VulkanShaderModule& shaderModule,
		const char* entryPointName = nullptr // defaults to "main"
	);

	VulkanGraphicsPipelineBuilder& AddFragmentShaderStage(
		const VulkanShaderModule& shaderModule,
		const char* entryPointName = nullptr // defaults to "main"
	);

	VulkanGraphicsPipelineBuilder& AddDynamicState(VkDynamicState dynamicState);

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

	static const auto MAX_DYNAMIC_STATE = 8;
	VkDynamicState dynamicState[MAX_DYNAMIC_STATE];
	int dynamicStateCount = 0;

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

	if (dynamicStateCount > 0) {
		VkPipelineDynamicStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		create_info.dynamicStateCount = dynamicStateCount;
		create_info.pDynamicStates = dynamicState;
		info.pDynamicState = &create_info;
	}

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

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddShaderStage(
	const VkPipelineShaderStageCreateInfo& stage)
{
	shaderStages.push_back(stage); // will perform copy
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddShaderStage(
	VkShaderStageFlagBits vkStageFlagBits, 
	VkShaderModule vkShaderModule, 
	const char* entryPointName, 
	VkSpecializationInfo* vkPSpecializationInfo)
{
	shaderStages.emplace_back();
	auto& stage = shaderStages[shaderStages.size() - 1];
	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = vkStageFlagBits;
	stage.module = vkShaderModule;
	stage.pName = entryPointName != nullptr ? entryPointName : "main";
	stage.pSpecializationInfo = nullptr;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddVertexShaderStage(const VulkanShaderModule& shaderModule, const char* entryPointName)
{
	return AddShaderStage(
		VK_SHADER_STAGE_VERTEX_BIT,
		shaderModule.shaderModule,
		entryPointName);
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddFragmentShaderStage(const VulkanShaderModule& shaderModule, const char* entryPointName)
{
	return AddShaderStage(
		VK_SHADER_STAGE_FRAGMENT_BIT,
		shaderModule.shaderModule,
		entryPointName);
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddDynamicState(VkDynamicState dynamicState)
{
	if (dynamicStateCount >= MAX_DYNAMIC_STATE) {
		throw std::runtime_error("exceeded maximum dynamic states supported by builder");
	}
	this->dynamicState[this->dynamicStateCount++] = dynamicState;
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
