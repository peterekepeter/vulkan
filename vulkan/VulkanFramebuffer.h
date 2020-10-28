#pragma once
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>

class VulkanFramebuffer
{
public: 
	VkFramebuffer m_vk_framebuffer = 0;
	VkDevice m_vk_device = VK_NULL_HANDLE;

	VulkanFramebuffer(){}

	VulkanFramebuffer(VkDevice device, const VkFramebufferCreateInfo &info)
		: m_vk_device(device) 
	{
		auto result = vkCreateFramebuffer(device, &info, nullptr, &m_vk_framebuffer);
		switch (result) {
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkCreateFramebuffer: failed, ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkCreateFramebuffer: failed, ran out of device memory");
		case VK_SUCCESS:
			break;
		}
	}

	~VulkanFramebuffer() 
	{
		if (m_vk_framebuffer != 0)
		{
			vkDestroyFramebuffer(m_vk_device, m_vk_framebuffer, nullptr);
		}
	}

	VulkanFramebuffer(const VulkanFramebuffer& other) = delete;
	VulkanFramebuffer& operator =(const VulkanFramebuffer& other) = delete;

	VulkanFramebuffer(VulkanFramebuffer&& other) noexcept
		: m_vk_device(other.m_vk_device)
		, m_vk_framebuffer(other.m_vk_framebuffer)
	{
		other.m_vk_framebuffer = 0;
	}

	VulkanFramebuffer& operator =(VulkanFramebuffer&& other) noexcept
	{
		if (m_vk_framebuffer != 0)
		{
			vkDestroyFramebuffer(m_vk_device, m_vk_framebuffer, nullptr);
		}
		m_vk_device = other.m_vk_device;
		m_vk_framebuffer = other.m_vk_framebuffer;
		other.m_vk_framebuffer = 0;
	}
	

	class Builder
	{
	public:
		VkDevice m_vk_device;
		VkFramebufferCreateInfo m_info;
		std::vector<VkImageView> m_attachments;

		Builder(VkDevice device)
			: m_vk_device(device)
		{
			m_info = {};
			m_info.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			m_info.pNext = nullptr;
			m_info.flags = 0;
			m_info.renderPass = VK_NULL_HANDLE;
			m_info.attachmentCount = VK_NULL_HANDLE;
			m_info.pAttachments = nullptr;
			m_info.width = 0;
			m_info.height = 0;
			m_info.layers = 1;
		}

		Builder& render_pass(VulkanRenderPass& render_pass) { return set_render_pass(render_pass); }
		Builder& attachment(VulkanImageView& render_pass) { return add_attachment(render_pass); }

		Builder& imageless() { return set_flags(VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT); }
		Builder& size(uint32_t width, uint32_t height) { return set_width(width).set_height(height); }
		Builder& add_attachment(VulkanImageView& image_view){ return add_attachment(image_view.vkImageViewHandle); }
		Builder& set_render_pass(VulkanRenderPass& render_pass) { return set_render_pass(render_pass.m_vk_render_pass); }

		Builder& set_width(uint32_t width) { m_info.width = width; return *this; }
		Builder& set_height(uint32_t height) { m_info.height = height; return *this; }
		Builder& set_render_pass(VkRenderPass render_pass) { m_info.renderPass = render_pass; return *this; }
		Builder& set_flags(VkFramebufferCreateFlags flags) { m_info.flags = flags; return *this; }
		Builder& add_attachment(VkImageView attachment) { m_attachments.push_back(attachment); m_info.pAttachments = m_attachments.data(); m_info.attachmentCount = m_attachments.size(); return *this; }
 
		VulkanFramebuffer build() {
			return VulkanFramebuffer(m_vk_device, m_info);
		}
		operator VulkanFramebuffer() {
			return build();
		}

	};
};