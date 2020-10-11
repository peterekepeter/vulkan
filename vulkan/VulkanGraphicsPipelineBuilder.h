#pragma once
#include "VulkanGraphicsPipeline.h"
#include <vector>


class VulkanGraphicsPipelineBuilder
{
public:

	VulkanGraphicsPipelineBuilder(VkDevice vkDevice);

	VulkanGraphicsPipeline Build();
	operator VulkanGraphicsPipeline();

	VulkanGraphicsPipelineBuilder& add_shader_stage(const VkPipelineShaderStageCreateInfo& );
	VulkanGraphicsPipelineBuilder& add_shader_stage(VkShaderStageFlagBits, VkShaderModule, const char* = "main", VkSpecializationInfo* = nullptr);

	VulkanGraphicsPipelineBuilder& add_vertex_shader(const VulkanShaderModule&, const char* entry = "main");
	VulkanGraphicsPipelineBuilder& add_fragment_shader(const VulkanShaderModule&, const char* entry = "main");
	VulkanGraphicsPipelineBuilder& add_dynamic_state(VkDynamicState);

	// shortcuts

	VulkanGraphicsPipelineBuilder& no_vertex_input();

	VulkanGraphicsPipelineBuilder& topology_points() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_POINT_LIST); }
	VulkanGraphicsPipelineBuilder& topology_lines() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_LINE_LIST); }
	VulkanGraphicsPipelineBuilder& topology_line_strip() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP); }
	VulkanGraphicsPipelineBuilder& topology_triangles() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); }
	VulkanGraphicsPipelineBuilder& topology_triangle_strip() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP); }
	VulkanGraphicsPipelineBuilder& topology_triangle_fan() { return set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN); }
	VulkanGraphicsPipelineBuilder& topology_enable_restart() { return set_input_assembly_primitive_restart(true); }
	VulkanGraphicsPipelineBuilder& viewport(uint32_t width, uint32_t height) { return set_viewport_size(width, height).set_viewport_position(0, 0).set_scissor_offset(0, 0).set_scissor_extent(width, height); }
	VulkanGraphicsPipelineBuilder& cull_none() { return set_cull_mode(VK_CULL_MODE_NONE); }
	VulkanGraphicsPipelineBuilder& cull_clockwise() { return set_cull_mode(VK_CULL_MODE_BACK_BIT).set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE); }
	VulkanGraphicsPipelineBuilder& cull_counter_clockwise() { return set_cull_mode(VK_CULL_MODE_BACK_BIT).set_front_face(VK_FRONT_FACE_CLOCKWISE); }
	VulkanGraphicsPipelineBuilder& polygon_mode_fill() { return set_polygon_mode(VK_POLYGON_MODE_FILL); }
	VulkanGraphicsPipelineBuilder& polygon_mode_line(float lineWidth = 1.0f) { return set_polygon_mode(VK_POLYGON_MODE_LINE).set_line_width(lineWidth); }
	VulkanGraphicsPipelineBuilder& polygon_mode_point() { return set_polygon_mode(VK_POLYGON_MODE_POINT); }

	VulkanGraphicsPipelineBuilder& no_blend() { return set_logic_blend_enable(false).set_color_blend_enable(false); }
	VulkanGraphicsPipelineBuilder& blend_classic_alpha() { return set_color_blend_enable(true).set_color_blend(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).set_alpha_blend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& blend_premultiplied_alpha() { return set_color_blend_enable(true).set_color_blend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).set_alpha_blend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& blend_soft_additive() { return set_color_blend_enable(true).set_color_blend(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA).set_alpha_blend(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }
	VulkanGraphicsPipelineBuilder& blend_additive() { return set_color_blend_enable(true).set_color_blend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE).set_alpha_blend(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE); }

	// setters

	VulkanGraphicsPipelineBuilder& set_input_assembly_primitive_restart(bool restart) { m_input_assembly_state.primitiveRestartEnable = restart ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_input_assembly(VkPrimitiveTopology topology) { m_input_assembly_state.topology = topology; return *this; }

	VulkanGraphicsPipelineBuilder& set_viewport_x(float x) { m_viewport.x = x; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_y(float y) { m_viewport.y = y; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_width(float width) { m_viewport.width = width; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_height(float height) { m_viewport.height = height; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_min_depth(float depth) { m_viewport.minDepth = depth; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_max_depth(float depth) { m_viewport.maxDepth = depth; return *this; }
	VulkanGraphicsPipelineBuilder& set_viewport_depth(float min, float max) { return set_viewport_min_depth(min).set_viewport_max_depth(max); }
	VulkanGraphicsPipelineBuilder& set_viewport_position(float x, float y) { return set_viewport_x(x).set_viewport_y(y); }
	VulkanGraphicsPipelineBuilder& set_viewport_size(float width, float height) { return set_viewport_width(width).set_viewport_height(height); }

	VulkanGraphicsPipelineBuilder& set_scissor_width(uint32_t width) { m_scissor.extent.width = width; return *this; }
	VulkanGraphicsPipelineBuilder& set_scissor_height(uint32_t height) { m_scissor.extent.height = height; return *this; }
	VulkanGraphicsPipelineBuilder& set_scissor_extent(uint32_t width, uint32_t height) { return set_scissor_width(width).set_scissor_height(height); }
	VulkanGraphicsPipelineBuilder& set_scissor_extent(VkExtent2D extent) { return set_scissor_width(extent.width).set_scissor_height(extent.height); }
	VulkanGraphicsPipelineBuilder& set_scissor_x(int32_t x) { m_scissor.offset.x = x; return *this; }
	VulkanGraphicsPipelineBuilder& set_scissor_y(int32_t y) { m_scissor.offset.y = y; return *this; }
	VulkanGraphicsPipelineBuilder& set_scissor_offset(int32_t x, int32_t y) { return set_scissor_x(x).set_scissor_y(y); }
	VulkanGraphicsPipelineBuilder& set_scissor_offset(VkOffset2D offset) { return set_scissor_x(offset.x).set_scissor_y(offset.y); }

	VulkanGraphicsPipelineBuilder& set_depth_clamp_enable(bool enable) { m_rasterizer_state.depthClampEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_rasterizer_discard_enable(bool enable) { m_rasterizer_state.rasterizerDiscardEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_polygon_mode(VkPolygonMode mode) { m_rasterizer_state.polygonMode = mode; return *this; }
	VulkanGraphicsPipelineBuilder& set_cull_mode(VkCullModeFlagBits flags) { m_rasterizer_state.cullMode = flags; return *this; }
	VulkanGraphicsPipelineBuilder& set_front_face(VkFrontFace face) { m_rasterizer_state.frontFace = face; return *this; }
	VulkanGraphicsPipelineBuilder& set_depth_bias_enable(bool enable) { m_rasterizer_state.depthBiasEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_depth_bias_constant_factor(float factor) { m_rasterizer_state.depthBiasConstantFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_depth_bias_clamp(float clamp) { m_rasterizer_state.depthBiasClamp = clamp; return *this; }
	VulkanGraphicsPipelineBuilder& set_depth_bias_slope_factor(float factor) { m_rasterizer_state.depthBiasSlopeFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_line_width(float width) { m_rasterizer_state.lineWidth = width; return *this; } 

	VulkanGraphicsPipelineBuilder& set_logic_blend_enable(bool enable) { m_color_blend_state.logicOpEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_logic_blend_op(VkLogicOp op) { m_color_blend_state.logicOp = op; return *this; }
	VulkanGraphicsPipelineBuilder& set_blend_constants(float c0, float c1, float c2, float c3) { m_color_blend_state.blendConstants[0] = c0; m_color_blend_state.blendConstants[1] = c1; m_color_blend_state.blendConstants[2] = c2; m_color_blend_state.blendConstants[3] = c3; return *this; }

	VulkanGraphicsPipelineBuilder& set_color_blend_enable(bool enable) { m_color_blend_attachment.blendEnable = enable ? VK_TRUE : VK_FALSE; return *this; }
	VulkanGraphicsPipelineBuilder& set_src_color_blend_factor(VkBlendFactor factor) { m_color_blend_attachment.srcColorBlendFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_dst_color_blend_factor(VkBlendFactor factor) { m_color_blend_attachment.dstColorBlendFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_src_alpha_blend_factor(VkBlendFactor factor) { m_color_blend_attachment.srcAlphaBlendFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_dst_alpha_blend_factor(VkBlendFactor factor) { m_color_blend_attachment.dstAlphaBlendFactor = factor; return *this; }
	VulkanGraphicsPipelineBuilder& set_color_blend_op(VkBlendOp op) { m_color_blend_attachment.colorBlendOp = op; return *this; }
	VulkanGraphicsPipelineBuilder& set_alpha_blend_op(VkBlendOp op) { m_color_blend_attachment.alphaBlendOp = op; return *this; }
	VulkanGraphicsPipelineBuilder& set_color_blend(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) { return set_src_color_blend_factor(src).set_color_blend_op(op).set_dst_color_blend_factor(dst); }
	VulkanGraphicsPipelineBuilder& set_alpha_blend(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) { return set_src_alpha_blend_factor(src).set_alpha_blend_op(op).set_dst_alpha_blend_factor(dst); }

	// from create info

	VulkanGraphicsPipelineBuilder& set_vertex_input_state(const VkPipelineVertexInputStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_input_assebly_state(const VkPipelineInputAssemblyStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_viewport_state(VkPipelineViewportStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_rasterizer_state(VkPipelineRasterizationStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_multisample_state(const VkPipelineMultisampleStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_color_blend_state(const VkPipelineColorBlendStateCreateInfo&);
	VulkanGraphicsPipelineBuilder& set_pipeline_layot(const VkPipelineLayout&);
	VulkanGraphicsPipelineBuilder& set_render_pass(const VkRenderPass&);
	VulkanGraphicsPipelineBuilder& set_subpass_index(uint32_t);


	// disallow copy
	VulkanGraphicsPipelineBuilder(const VulkanGraphicsPipelineBuilder&) = delete;
	VulkanGraphicsPipelineBuilder& operator =(const VulkanGraphicsPipelineBuilder&) = delete;
	// allow move
	VulkanGraphicsPipelineBuilder(VulkanGraphicsPipelineBuilder&&) = default;
	VulkanGraphicsPipelineBuilder& operator =(VulkanGraphicsPipelineBuilder&&) = default;


private:
	VkDevice m_vk_device = nullptr;
	
	// state
	std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
	VkPipelineVertexInputStateCreateInfo m_vertex_input_state;
	VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state;
	VkPipelineViewportStateCreateInfo m_viewport_state;
	VkPipelineRasterizationStateCreateInfo m_rasterizer_state;
	VkPipelineMultisampleStateCreateInfo m_multisample_state;
	VkPipelineColorBlendStateCreateInfo m_color_blend_state;

	VkViewport m_viewport;
	VkRect2D m_scissor;
	VkPipelineColorBlendAttachmentState m_color_blend_attachment;

	std::vector<VkDynamicState> m_dynamic_state;

	// renderpass
	VkPipelineLayout m_vk_pipeline_layout;
	VkRenderPass m_vk_render_pass;
	uint32_t m_subpass_index;
};


inline VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(VkDevice vkDevice)
	: m_vk_device(vkDevice)
{
	// initialize structures and add sensible defaults
	m_vertex_input_state = {};
	m_vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_input_assembly_state = {};
	m_input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_input_assembly_state.primitiveRestartEnable = VK_FALSE;
	m_viewport_state = {};
	m_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewport_state.viewportCount = 1;
	m_viewport_state.pViewports = &m_viewport;
	m_viewport_state.scissorCount = 1;
	m_viewport_state.pScissors = &m_scissor;
	m_viewport = {};
	m_viewport.maxDepth = 1.0f;
	m_scissor = {};
	m_rasterizer_state = {};
	m_rasterizer_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizer_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_rasterizer_state.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizer_state.cullMode = VK_CULL_MODE_BACK_BIT;
	m_rasterizer_state.lineWidth = 1.0f;
	m_multisample_state = {};
	m_multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisample_state.minSampleShading = 1.0f;
	m_color_blend_state = {};
	m_color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_color_blend_state.attachmentCount = 1;
	m_color_blend_state.pAttachments = &m_color_blend_attachment;
	m_color_blend_state.blendConstants[0] = 0.0f;
	m_color_blend_state.blendConstants[1] = 0.0f;
	m_color_blend_state.blendConstants[2] = 0.0f;
	m_color_blend_state.blendConstants[3] = 0.0f;
	m_color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_color_blend_attachment.blendEnable = VK_FALSE;
	m_subpass_index = 0;
	m_vk_pipeline_layout = VK_NULL_HANDLE;
	m_vk_render_pass = VK_NULL_HANDLE;
}

inline VulkanGraphicsPipeline VulkanGraphicsPipelineBuilder::Build()
{
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.stageCount = m_shader_stages.size();
	info.pStages = m_shader_stages.data();
	info.pVertexInputState = &m_vertex_input_state;
	info.pInputAssemblyState = &m_input_assembly_state;
	info.pViewportState = &m_viewport_state;
	info.pRasterizationState = &m_rasterizer_state;
	info.pMultisampleState = &m_multisample_state;
	info.pDepthStencilState = nullptr; // Optional
	info.pColorBlendState = &m_color_blend_state;
	info.pDynamicState = nullptr; // Optional

	if (m_dynamic_state.size() > 0) {
		VkPipelineDynamicStateCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		create_info.dynamicStateCount = m_dynamic_state.size();
		create_info.pDynamicStates = m_dynamic_state.data();
		info.pDynamicState = &create_info;
	}

	info.layout = m_vk_pipeline_layout;
	info.renderPass = m_vk_render_pass;
	info.subpass = m_subpass_index;

	info.basePipelineHandle = VK_NULL_HANDLE; // Optional
	info.basePipelineIndex = -1; // Optional
	return VulkanGraphicsPipeline(m_vk_device, info);
}

inline VulkanGraphicsPipelineBuilder::operator VulkanGraphicsPipeline()
{
	return Build();
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::add_shader_stage(
	const VkPipelineShaderStageCreateInfo& stage)
{
	m_shader_stages.push_back(stage); // will perform copy
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::add_shader_stage(
	VkShaderStageFlagBits vkStageFlagBits, 
	VkShaderModule vkShaderModule, 
	const char* entryPointName, 
	VkSpecializationInfo* vkPSpecializationInfo)
{
	m_shader_stages.emplace_back();
	auto& stage = m_shader_stages[m_shader_stages.size() - 1];
	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = vkStageFlagBits;
	stage.module = vkShaderModule;
	stage.pName = entryPointName != nullptr ? entryPointName : "main";
	stage.pSpecializationInfo = nullptr;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::add_vertex_shader(const VulkanShaderModule& shaderModule, const char* entryPointName)
{
	return add_shader_stage(
		VK_SHADER_STAGE_VERTEX_BIT,
		shaderModule.shaderModule,
		entryPointName);
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::add_fragment_shader(const VulkanShaderModule& shaderModule, const char* entryPointName)
{
	return add_shader_stage(
		VK_SHADER_STAGE_FRAGMENT_BIT,
		shaderModule.shaderModule,
		entryPointName);
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::add_dynamic_state(VkDynamicState state)
{
	m_dynamic_state.push_back(state);
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::no_vertex_input()
{
	m_vertex_input_state.vertexBindingDescriptionCount = 0;
	m_vertex_input_state.pVertexBindingDescriptions = nullptr; 
	m_vertex_input_state.vertexAttributeDescriptionCount = 0;
	m_vertex_input_state.pVertexAttributeDescriptions = nullptr;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_vertex_input_state(const VkPipelineVertexInputStateCreateInfo& state)
{
	m_vertex_input_state = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_input_assebly_state(const VkPipelineInputAssemblyStateCreateInfo& state)
{
	m_input_assembly_state = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_viewport_state(VkPipelineViewportStateCreateInfo& state) {
	m_viewport_state = state; // has pointers
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_rasterizer_state(VkPipelineRasterizationStateCreateInfo& state)
{
	m_rasterizer_state = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_multisample_state(const VkPipelineMultisampleStateCreateInfo& state)
{
	m_multisample_state = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_color_blend_state(const VkPipelineColorBlendStateCreateInfo& state)
{
	m_color_blend_state = state;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_pipeline_layot(const VkPipelineLayout& pipelineLayout)
{
	m_vk_pipeline_layout = pipelineLayout;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_render_pass(const VkRenderPass& renderPass)
{
	m_vk_render_pass = renderPass;
	return *this;
}

inline VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::set_subpass_index(uint32_t subpassIndex)
{
	m_subpass_index = subpassIndex;
	return *this;
}
