#pragma once
#include <vulkan/vulkan_core.h>


class VulkanSampler
{
public:
	VkDevice m_vk_device;
	VkSampler m_vk_sampler;

	VulkanSampler()
		: m_vk_device(VK_NULL_HANDLE)
		, m_vk_sampler(VK_NULL_HANDLE)
	{}

	VulkanSampler(VkDevice device, const VkSamplerCreateInfo& info)
		: m_vk_device(device)
		, m_vk_sampler(VK_NULL_HANDLE)
	{
		vkCreateSampler(m_vk_device, &info, nullptr, &m_vk_sampler);
	}

	VulkanSampler(const VulkanSampler& other) = delete;
	VulkanSampler& operator =(const VulkanSampler& other) = delete;

	VulkanSampler(VulkanSampler&& other) noexcept
		: m_vk_device(other.m_vk_device)
		, m_vk_sampler(other.m_vk_sampler)
	{
		other.m_vk_sampler = VK_NULL_HANDLE;
	}

	VulkanSampler& operator =(VulkanSampler&& other) noexcept
	{
		if (m_vk_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(m_vk_device, m_vk_sampler, nullptr);
		}
		m_vk_device = other.m_vk_device;
		m_vk_sampler = other.m_vk_sampler;
		other.m_vk_sampler = VK_NULL_HANDLE;
	}

	~VulkanSampler() {
		if (m_vk_sampler != VK_NULL_HANDLE) {
			vkDestroySampler(m_vk_device, m_vk_sampler, nullptr);
		}
	}

	class Builder
	{
	public:
		VkDevice m_vk_device;
		VkSamplerCreateInfo m_info;

		operator VulkanSampler() {
			return VulkanSampler(m_vk_device, m_info);
		}

		VulkanSampler build() {
			return VulkanSampler(m_vk_device, m_info);
		}

		Builder(VkDevice device)
			: m_vk_device(device) {
			m_info = {};
			m_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			m_info.magFilter = VK_FILTER_LINEAR;
			m_info.minFilter = VK_FILTER_LINEAR;
			m_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			m_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			m_info.mipLodBias = 0.0f;
			m_info.anisotropyEnable = VK_FALSE;
			m_info.maxAnisotropy = 1.0f;
			m_info.compareEnable = VK_FALSE;
			m_info.minLod = 0.0f;
			m_info.maxLod = 16.0f;
			m_info.compareOp = VK_COMPARE_OP_NEVER;
			m_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			m_info.unnormalizedCoordinates = VK_TRUE;
		}

		Builder& unnormalized_coordinates() { return set_unnormalized_coordinates(true).mip_mode_nearest().lod_range(0.0f, 0.0f).address_mode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE); }
		Builder& use_anisotropy(float max) { return set_anisotropy_enable(true).set_max_anisotropy(max); }
		Builder& no_anisotropy() { return set_anisotropy_enable(false); }
		Builder& lod_bias(float bias) { return set_mip_lod_bias(bias); }
		Builder& lod_range(float min, float max) { return set_min_lod(min).set_max_lod(max); }
		Builder& mip_mode_nearest() { return set_mipmap_mode(VK_SAMPLER_MIPMAP_MODE_NEAREST); }
		Builder& mip_mode_linear() { return set_mipmap_mode(VK_SAMPLER_MIPMAP_MODE_LINEAR); }
		Builder& filter_nearest() { return filter(VK_FILTER_NEAREST); }
		Builder& filter_linear() { return filter(VK_FILTER_LINEAR); }
		Builder& filter(VkFilter filter) { return set_mag_filter(filter).set_min_filter(filter); }
		Builder& address_mode(VkSamplerAddressMode value) { return set_address_mode_u(value).set_address_mode_v(value).set_address_mode_w(value); }

		Builder& set_mag_filter(VkFilter value) { m_info.magFilter = value; return *this; }
		Builder& set_min_filter(VkFilter value) { m_info.minFilter = value; return *this; }
		Builder& set_mipmap_mode(VkSamplerMipmapMode value) { m_info.mipmapMode = value; return *this; }
		Builder& set_address_mode_u(VkSamplerAddressMode value) { m_info.addressModeU = value; return *this; }
		Builder& set_address_mode_v(VkSamplerAddressMode value) { m_info.addressModeV = value; return *this; }
		Builder& set_address_mode_w(VkSamplerAddressMode value) { m_info.addressModeW = value; return *this; }
		Builder& set_mip_lod_bias(float value) { m_info.mipLodBias = value; return *this; }
		Builder& set_anisotropy_enable(float value) { m_info.anisotropyEnable = value ? VK_TRUE : VK_FALSE; return *this; }
		Builder& set_max_anisotropy(float value) { m_info.maxAnisotropy = value; return *this; }
		Builder& set_compare_enable(float value) { m_info.compareEnable = value ? VK_TRUE : VK_FALSE; return *this; }
		Builder& set_min_lod(float value) { m_info.minLod = value; return *this; }
		Builder& set_max_lod(float value) { m_info.maxLod = value; return *this; }
		Builder& set_compare_op(VkCompareOp value) { m_info.compareOp = value; return *this; }
		Builder& set_border_color(VkBorderColor value) { m_info.borderColor = value; return *this; }
		Builder& set_unnormalized_coordinates(bool value) { m_info.unnormalizedCoordinates = value ? VK_TRUE : VK_FALSE; return *this; }


	};
};