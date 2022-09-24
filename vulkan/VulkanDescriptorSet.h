#pragma once
#include "VulkanSampler.h"
#include "VulkanImageView.h"

// allocated and freed by VulkanDescriptorPool
class VulkanDescriptorSet
{
	DECLARE_DEFAULT_MOVEABLE_COPYABLE_TYPE(VulkanDescriptorSet)
public:
	VkDevice m_vk_device;
	VkDescriptorSet m_vk_descriptor_set;

	VulkanDescriptorSet() 
		: m_vk_device(nullptr)
		, m_vk_descriptor_set(0) 
	{
		
	}

	VulkanDescriptorSet& write_uniform_buffer(
		uint32_t binding, 
		VkBuffer buffer, 
		VkDeviceSize offset = 0, 
		VkDeviceSize range = VK_WHOLE_SIZE) 
	{
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = buffer;
		buffer_info.offset = offset;
		buffer_info.range = range;
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vk_descriptor_set;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &buffer_info;
		write.pImageInfo = nullptr; // Optional
		write.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(m_vk_device, 1, &write, 0, nullptr);
		return *this;
	}

	VulkanDescriptorSet& write_image_sampler(
		uint32_t binding,
		const VulkanSampler& sampler,
		const VulkanImageView& image_view,
		VkImageLayout vk_image_layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		return write_image_sampler(
			binding,
			sampler.m_vk_sampler,
			image_view.m_vk_image_view,
			vk_image_layout
		);
	}
	VulkanDescriptorSet& write_image_sampler(
		uint32_t binding, 
		VkSampler vk_sampler, 
		VkImageView vk_image_view, 
		VkImageLayout vk_image_layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		VkDescriptorImageInfo image_info = {};
		image_info.sampler = vk_sampler;
		image_info.imageView = vk_image_view;
		image_info.imageLayout = vk_image_layout;
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vk_descriptor_set;
		write.dstBinding = binding;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pBufferInfo = nullptr;
		write.pImageInfo = &image_info; // Optional
		write.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(m_vk_device, 1, &write, 0, nullptr);
		return *this;
	}

	VulkanDescriptorSet(VkDevice vk_device, VkDescriptorSetAllocateInfo& info) 
	{
		m_vk_device = vk_device;
		switch (vkAllocateDescriptorSets(vk_device, &info, &m_vk_descriptor_set)) {
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkAllocateDescriptorSets: failed, ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkAllocateDescriptorSets: failed, ran out of device memory");
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			throw std::runtime_error("vkAllocateDescriptorSets: failed, ran out of pool memory");
		case VK_ERROR_FRAGMENTED_POOL:
			throw std::runtime_error("vkAllocateDescriptorSets: failed, fragmented pool");
		case VK_SUCCESS:
			break;
		}
	}
};