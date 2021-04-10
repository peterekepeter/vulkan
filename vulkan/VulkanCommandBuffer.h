#pragma once
#include <vulkan/vulkan_core.h>
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"

// allocated and freed by VulkanCommandPool
class VulkanCommandBuffer
{
	DECLARE_DEFAULT_MOVEABLE_COPYABLE_TYPE(VulkanCommandBuffer)
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
		VulkanRenderPass& render_pass,
		VulkanFramebuffer& frame_buffer,
		const VkRect2D& render_area,
		uint32_t clear_value_count = 0,
		VkClearValue* clear_values = nullptr)
	{
		return begin_render_pass(
			render_pass.m_vk_render_pass, 
			frame_buffer.m_vk_framebuffer, 
			render_area, 
			clear_value_count, 
			clear_values);
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

	VulkanCommandBuffer& clear_attachment(uint32_t attachment, VkImageAspectFlags aspect_mask, const VkClearValue& clear_value, uint32_t width, uint32_t height, int32_t x = 0, int32_t y = 0) {
		VkClearAttachment clear;
		clear.aspectMask = aspect_mask;
		clear.clearValue = clear_value;
		clear.colorAttachment = attachment;
		VkClearRect rect;
		rect.baseArrayLayer = 0;
		rect.layerCount = 1;
		rect.rect.extent.width = width;
		rect.rect.extent.height = height;
		rect.rect.offset.x = x;
		rect.rect.offset.y = y;
		vkCmdClearAttachments(m_vk_command_buffer, 1, &clear, 1, &rect);
		return *this;
	}

	VulkanCommandBuffer& clear_color_image(VkImage image, VkImageLayout image_layout, const VkClearColorValue& clear_color)
	{
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = 1;
		range.levelCount = 1;
		vkCmdClearColorImage(m_vk_command_buffer, image, image_layout, &clear_color, 1, &range);
		return *this;
	}

	VulkanCommandBuffer& copy_image(VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, 
		int32_t width, int32_t height, int32_t src_x = 0, int32_t src_y = 0, int32_t dst_x = 0, int32_t dst_y = 0) {
		VkImageCopy copy;
		copy = {};
		copy.srcOffset.x = src_x;
		copy.srcOffset.y = src_y;
		copy.dstOffset.x = dst_x;
		copy.dstOffset.y = dst_y;
		copy.extent.width = width;
		copy.extent.height = height;
		copy.extent.depth = 1;
		copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.srcSubresource.baseArrayLayer = 0;
		copy.srcSubresource.layerCount = 1;
		copy.srcSubresource.mipLevel = 0;
		copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.dstSubresource.baseArrayLayer = 0;
		copy.dstSubresource.layerCount = 1;
		copy.dstSubresource.mipLevel = 0;

		vkCmdCopyImage(m_vk_command_buffer, src, src_layout, dst, dst_layout, 1, &copy);
		return *this;
	}

	class Barrier
	{
	public:
		VkPipelineStageFlags m_source_mask;
		VkPipelineStageFlags m_dest_mask; 
		VkDependencyFlagBits m_dependency_flags;
		VulkanCommandBuffer& m_parent;

		std::vector<VkMemoryBarrier> m_memory_barriers;
		std::vector<VkBufferMemoryBarrier> m_buffer_barriers;
		std::vector<VkImageMemoryBarrier> m_image_memory_barriers;

		Barrier(
			VulkanCommandBuffer& parent,
			VkPipelineStageFlags src_mask, 
			VkPipelineStageFlags dest_mask, 
			VkDependencyFlagBits dependency_flags) 
			: m_parent(parent)
			, m_source_mask(src_mask)
			, m_dest_mask(dest_mask)
			, m_dependency_flags(dependency_flags)
		{

		}

		Barrier& image(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImage image, 
			VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) 
		{
			m_image_memory_barriers.emplace_back();
			auto& image_barrier = m_image_memory_barriers.back();
			image_barrier = {};
			image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_barrier.srcAccessMask = srcAccessMask;
			image_barrier.dstAccessMask = dstAccessMask;
			image_barrier.oldLayout = oldLayout;
			image_barrier.newLayout = newLayout;
			image_barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
			image_barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
			image_barrier.image = image;
			image_barrier.subresourceRange.aspectMask = aspectMask;
			image_barrier.subresourceRange.baseMipLevel = baseMipLevel;
			image_barrier.subresourceRange.levelCount = levelCount;
			image_barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
			image_barrier.subresourceRange.layerCount = layerCount;
			return *this;
		}

		VulkanCommandBuffer& end_barrier() {
			vkCmdPipelineBarrier(
				m_parent.m_vk_command_buffer, 
				m_source_mask, 
				m_dest_mask, 
				m_dependency_flags,
				m_memory_barriers.size(),
				m_memory_barriers.data(),
				m_buffer_barriers.size(),
				m_buffer_barriers.data(),
				m_image_memory_barriers.size(),
				m_image_memory_barriers.data()
			);
			return m_parent;
		}
	};

	Barrier begin_barrier(
		VkPipelineStageFlags src_mask, 
		VkPipelineStageFlags dest_mask, 
		VkDependencyFlagBits dependency_flags)
	{
		return Barrier(*this, src_mask, dest_mask, dependency_flags);
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