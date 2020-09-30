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

	// shortcuts

	VulkanGraphicsPipelineBuilder& NoVertexInput();

	VulkanGraphicsPipelineBuilder& AssemblyTriangleList() { return SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); }
	VulkanGraphicsPipelineBuilder& AssemblyEnableRestart() { return SetInputAssemblyPrimitiveRestart(true); }
	VulkanGraphicsPipelineBuilder& Viewport(uint32_t width, uint32_t height) { return SetViewportSize(width, height).SetViewportPosition(0, 0).SetScissorOffset(0, 0).SetScissorExtent(width, height); }
	VulkanGraphicsPipelineBuilder& CullNone() { return SetCullMode(VK_CULL_MODE_NONE); }
	VulkanGraphicsPipelineBuilder& CullClockwise() { return SetCullMode(VK_CULL_MODE_BACK_BIT).SetFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE); }
	VulkanGraphicsPipelineBuilder& CullCounterClockwise() { return SetCullMode(VK_CULL_MODE_BACK_BIT).SetFrontFace(VK_FRONT_FACE_CLOCKWISE); }
	VulkanGraphicsPipelineBuilder& PolygonModeFill() { return SetPolygonMode(VK_POLYGON_MODE_FILL); }
	VulkanGraphicsPipelineBuilder& PolygonModeLine(float lineWidth = 1.0f) { return SetPolygonMode(VK_POLYGON_MODE_LINE).SetLineWidth(lineWidth); }
	VulkanGraphicsPipelineBuilder& PolygonModePoint() { return SetPolygonMode(VK_POLYGON_MODE_POINT); }

	VulkanGraphicsPipelineBuilder& NoBlend() { return SetLogicBlendEnable(false).SetColorBlendEnable(false); }
	VulkanGraphicsPipelineBuilder& BlendClassicAlpha() { return SetColorBlendEnable(true).SetColorBlend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).SetAlphaBlend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& BlendPremultipliedAlpha() { return SetColorBlendEnable(true).SetColorBlend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).SetAlphaBlend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& BlendSoftAdditive() { return SetColorBlendEnable(true).SetColorBlend(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA).SetAlphaBlend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& BlendAdditive() { return SetColorBlendEnable(true).SetColorBlend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE).SetAlphaBlend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }

	// setters

	VulkanGraphicsPipelineBuilder& SetInputAssemblyPrimitiveRestart(bool restart) { inputAssemblyState.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetInputAssembly(VkPrimitiveTopology topology) { inputAssemblyState.topology = topology; return *this; }

	VulkanGraphicsPipelineBuilder& SetViewportX(float x) { viewport.x = x; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportY(float y) { viewport.y = y; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportWidth(float width) { viewport.width = width; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportHeight(float height) { viewport.height = height; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportMinDepth(float depth) { viewport.minDepth = depth; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportMaxDepth(float depth) { viewport.maxDepth = depth; return *this; }
	VulkanGraphicsPipelineBuilder& SetViewportDepth(float min, float max) { return SetViewportMinDepth(min).SetViewportMaxDepth(max); }
	VulkanGraphicsPipelineBuilder& SetViewportPosition(float x, float y) { return SetViewportX(x).SetViewportY(y); }
	VulkanGraphicsPipelineBuilder& SetViewportSize(float width, float height) { return SetViewportWidth(width).SetViewportHeight(height); }

	VulkanGraphicsPipelineBuilder& SetScissorWidth(uint32_t width) { scissor.extent.width = width; return *this; }
	VulkanGraphicsPipelineBuilder& SetScissorHeight(uint32_t height) { scissor.extent.height = height; return *this; }
	VulkanGraphicsPipelineBuilder& SetScissorExtent(uint32_t width, uint32_t height) { return SetScissorWidth(width).SetScissorHeight(height); }
	VulkanGraphicsPipelineBuilder& SetScissorExtent(VkExtent2D extent) { return SetScissorWidth(extent.width).SetScissorHeight(extent.height); }
	VulkanGraphicsPipelineBuilder& SetScissorX(int32_t x) { scissor.offset.x = x; return *this; }
	VulkanGraphicsPipelineBuilder& SetScissorY(int32_t y) { scissor.offset.y = y; return *this; }
	VulkanGraphicsPipelineBuilder& SetScissorOffset(int32_t x, int32_t y) { return SetScissorX(x).SetScissorY(y); }
	VulkanGraphicsPipelineBuilder& SetScissorOffset(VkOffset2D offset) { return SetScissorX(offset.x).SetScissorY(offset.y); }

	VulkanGraphicsPipelineBuilder& SetDepthClampEnable(bool enable) { rasterizerState.depthClampEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetRasterizerDiscardEnable(bool enable) { rasterizerState.rasterizerDiscardEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetPolygonMode(VkPolygonMode mode) { rasterizerState.polygonMode = mode; return *this; }
	VulkanGraphicsPipelineBuilder& SetCullMode(VkCullModeFlagBits cullModeFlags) { rasterizerState.cullMode = cullModeFlags; return *this; }
	VulkanGraphicsPipelineBuilder& SetFrontFace(VkFrontFace frontFace) { rasterizerState.frontFace = frontFace; return *this; }
	VulkanGraphicsPipelineBuilder& SetDepthBiasEnable(bool enable) { rasterizerState.depthBiasEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetDepthBiasConstantFactor(float depthBiasConstantFactor) { rasterizerState.depthBiasConstantFactor = depthBiasConstantFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetDepthBiasClamp(float depthBiasClamp) { rasterizerState.depthBiasClamp = depthBiasClamp; return *this; }
	VulkanGraphicsPipelineBuilder& SetDepthBiasSlopeFactor(float depthBiasSlopeFactor) { rasterizerState.depthBiasSlopeFactor = depthBiasSlopeFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetLineWidth(float lineWidth) { rasterizerState.lineWidth = lineWidth; return *this; } 

	VulkanGraphicsPipelineBuilder& SetLogicBlendEnable(bool enable) { colorBlendState.logicOpEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetLogicBlendOp(VkLogicOp logicOp) { colorBlendState.logicOp = logicOp; return *this; }
	VulkanGraphicsPipelineBuilder& SetBlendConstants(float c0, float c1, float c2, float c3) { colorBlendState.blendConstants[0] = c0; colorBlendState.blendConstants[1] = c1; colorBlendState.blendConstants[2] = c2; colorBlendState.blendConstants[3] = c3; return *this; }

	VulkanGraphicsPipelineBuilder& SetColorBlendEnable(bool enable) { colorBlendAttachment.blendEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& SetSrcColorBlendFactor(VkBlendFactor blendFactor) { colorBlendAttachment.srcColorBlendFactor = blendFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetDstColorBlendFactor(VkBlendFactor blendFactor) { colorBlendAttachment.dstColorBlendFactor = blendFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetSrcAlphaBlendFactor(VkBlendFactor blendFactor) { colorBlendAttachment.srcAlphaBlendFactor = blendFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetDstAlphaBlendFactor(VkBlendFactor blendFactor) { colorBlendAttachment.dstAlphaBlendFactor = blendFactor; return *this; }
	VulkanGraphicsPipelineBuilder& SetColorBlendOp(VkBlendOp blendOp) { colorBlendAttachment.colorBlendOp = blendOp; return *this; }
	VulkanGraphicsPipelineBuilder& SetAlphaBlendOp(VkBlendOp blendOp) { colorBlendAttachment.alphaBlendOp = blendOp; return *this; }
	VulkanGraphicsPipelineBuilder& SetColorBlend(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) { return SetSrcColorBlendFactor(src).SetColorBlendOp(op).SetDstColorBlendFactor(dst); }
	VulkanGraphicsPipelineBuilder& SetAlphaBlend(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) { return SetSrcAlphaBlendFactor(src).SetAlphaBlendOp(op).SetDstAlphaBlendFactor(dst); }

	// from create info

	VulkanGraphicsPipelineBuilder& SetVertexInputState(const VkPipelineVertexInputStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetViewportState(VkPipelineViewportStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetRasterizerState(VkPipelineRasterizationStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetMultisampleState(const VkPipelineMultisampleStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetColorBlendState(const VkPipelineColorBlendStateCreateInfo& state);
	VulkanGraphicsPipelineBuilder& SetPipelineLayout(const VkPipelineLayout& pipelineLayout);
	VulkanGraphicsPipelineBuilder& SetRenderPass(const VkRenderPass& renderPass);
	VulkanGraphicsPipelineBuilder& SetSubpassIndex(uint32_t subpassIndex);


	// disallow copy & move
	VulkanGraphicsPipelineBuilder(const VulkanGraphicsPipelineBuilder& other) = delete;
	VulkanGraphicsPipelineBuilder& operator =(const VulkanGraphicsPipelineBuilder& other) = delete;
	VulkanGraphicsPipelineBuilder(VulkanGraphicsPipelineBuilder&& other) = delete;
	VulkanGraphicsPipelineBuilder& operator =(VulkanGraphicsPipelineBuilder&& other) = delete;


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

	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;

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
	// initialize structures and add sensible defaults
	vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewport = {};
	viewport.maxDepth = 1.0f;
	scissor = {};
	rasterizerState = {};
	rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerState.lineWidth = 1.0f;
	multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;
	colorBlendState.blendConstants[0] = 0.0f;
	colorBlendState.blendConstants[1] = 0.0f;
	colorBlendState.blendConstants[2] = 0.0f;
	colorBlendState.blendConstants[3] = 0.0f;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
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

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::NoVertexInput()
{
	vertexInputState.vertexBindingDescriptionCount = 0;
	vertexInputState.pVertexBindingDescriptions = nullptr; 
	vertexInputState.vertexAttributeDescriptionCount = 0;
	vertexInputState.pVertexAttributeDescriptions = nullptr;
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
