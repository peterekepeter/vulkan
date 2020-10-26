#pragma once
#include <vulkan/vulkan_core.h>

class VulkanCommandBuffer
{
public:
	VkCommandBuffer m_vk_command_buffer;

	VulkanCommandBuffer() : m_vk_command_buffer(VK_NULL_HANDLE) { }
	VulkanCommandBuffer(VkCommandBuffer buffer) : m_vk_command_buffer(buffer) { }

	// command recording shortcuts

	VulkanCommandBuffer& begin_recording() { return begin_recording((VkCommandBufferUsageFlagBits)0); }
	VulkanCommandBuffer& begin_recording_one_shot() { return begin_recording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT); }
	VulkanCommandBuffer& begin_recording_simultaneous() { return begin_recording(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT); }

	VulkanCommandBuffer& bind_graphics_pipeline(const VulkanGraphicsPipeline& pipeline) { return bind_graphics_pipeline(pipeline.m_vk_pipeline); }
	VulkanCommandBuffer& bind_graphics_pipeline(VkPipeline vk_pipeline) { return bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline); }

	// command recording implementation

	VulkanCommandBuffer& begin_recording(VkCommandBufferUsageFlagBits flags) {
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		info.pInheritanceInfo = nullptr;
		auto vk_result = vkBeginCommandBuffer(m_vk_command_buffer, &info);
		switch (vk_result) {
		case VK_SUCCESS: 
			return *this;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkBeginCommandBuffer failed: ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkBeginCommandBuffer failed: ran out of device memory");
		default:
			throw std::runtime_error("vkBeginCommandBuffer failed: undefined behaviour");
		}
	}

	VulkanCommandBuffer& end_recording() {
		auto vk_result = vkEndCommandBuffer(m_vk_command_buffer);
		switch (vk_result) 
		{
		case VK_SUCCESS:
			return *this;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkEndCommandBuffer failed: ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkEndCommandBuffer failed: ran out of device memory");
		default:
			throw std::runtime_error("vkEndCommandBuffer failed: undefined behaviour");
		}
	}

	VulkanCommandBuffer& begin_render_pass(
		VkRenderPass& vk_render_pass, 
		VkFramebuffer& vk_frame_buffer, 
		const VkRect2D& render_area, 
		uint32_t clear_value_count = 0, 
		VkClearValue* clear_values = nullptr) {

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vk_render_pass;
		renderPassInfo.framebuffer = vk_frame_buffer;
		renderPassInfo.renderArea = render_area;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_vk_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		return *this;
	}

	VulkanCommandBuffer& end_render_pass() {
		vkCmdEndRenderPass(m_vk_command_buffer);
		return *this;
	}

	VulkanCommandBuffer& bind_pipeline(VkPipelineBindPoint vk_pipeline_bind_point, VkPipeline vk_pipeline) {
		vkCmdBindPipeline(m_vk_command_buffer, vk_pipeline_bind_point, vk_pipeline);
		return *this;
	}

	VulkanCommandBuffer& bind_graphics_descriptor_set(const VulkanPipelineLayout& pipeline_layout, VulkanDescriptorSet descriptor_set) {
		return bind_graphics_descriptor_set(pipeline_layout.m_vk_pipeline_layout, descriptor_set.m_vk_descriptor_set);
	}

	VulkanCommandBuffer& bind_graphics_descriptor_set(VkPipelineLayout vk_pipeline_layout, VkDescriptorSet vk_descriptor_set) {
		vkCmdBindDescriptorSets(
			m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 
			0, 1, &vk_descriptor_set, 
			0, nullptr);
		return *this;
	}

	VulkanCommandBuffer& set_viewport(const VkViewport& viewport)
	{
		vkCmdSetViewport(m_vk_command_buffer, 0, 1, &viewport);
		return *this;
	}

	VulkanCommandBuffer& draw(
		uint32_t vertex_count = 3, 
		uint32_t instance_count = 1, 
		uint32_t first_vertex = 0, 
		uint32_t first_instance = 0) {
		vkCmdDraw(m_vk_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
		return *this;
	}
};