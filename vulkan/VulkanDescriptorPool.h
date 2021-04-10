#pragma once
#include <vulkan\vulkan_core.h>
#include <stdexcept>
#include <vector>
#include "VulkanDescriptorSet.h"


class VulkanDescriptorPool
{
	DECLARE_MOVEABLE_COPYABLE_TYPE(VulkanDescriptorPool)
public:
	VkDevice m_vk_device;
	VkDescriptorPool m_descriptor_pool;

	VulkanDescriptorSet allocate_descriptor_set(const VulkanDescriptorSetLayout& layout);
	VulkanDescriptorSet allocate_descriptor_set(const VkDescriptorSetLayout& vk_layout);
	void free_descriptor_set(const VulkanDescriptorSet& descriptor_set);
	void free_descriptor_set(const VkDescriptorSet& vk_descriptor_set);

	class Builder
	{
	public:
		VkDevice m_vk_device;
		std::vector<VkDescriptorPoolSize> m_pool_sizes;

		Builder(VkDevice device) : m_vk_device(device) {}
		operator VulkanDescriptorPool();
		VulkanDescriptorPool build() { return operator VulkanDescriptorPool(); }

		// shortcuts

		Builder& samplers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_SAMPLER, count); }
		Builder& combined_image_samplers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count); }
		Builder& sampled_images(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count); }
		Builder& storage_images(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count); }
		Builder& uniform_texel_buffers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count); }
		Builder& storage_texel_buffers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count); }
		Builder& uniform_buffers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count); }
		Builder& storage_buffers(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count); }
		Builder& uniform_buffers_dynamic(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count); }
		Builder& storage_buffers_dynamic(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count); }
		Builder& input_attachments(uint32_t count) { return set(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count); }

		Builder& add_samplers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_SAMPLER, count); }
		Builder& add_combined_image_samplers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count); }
		Builder& add_sampled_images(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count); }
		Builder& add_storage_images(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count); }
		Builder& add_uniform_texel_buffers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count); }
		Builder& add_storage_texel_buffers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count); }
		Builder& add_uniform_buffers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count); }
		Builder& add_storage_buffers(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count); }
		Builder& add_uniform_buffers_dynamic(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count); }
		Builder& add_storage_buffers_dynamic(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count); }
		Builder& add_input_attachments(uint32_t count) { return add(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count); }

		// base

		Builder& add(VkDescriptorType type, uint32_t count)
		{
			for (auto& size : m_pool_sizes) {
				if (size.type == type)
				{
					size.descriptorCount += count;
					return *this; // found
				}
			}
			// not found
			m_pool_sizes.push_back({ type, count });
			return *this;
		}

		Builder& set(VkDescriptorType type, uint32_t count)
		{
			for (auto& size : m_pool_sizes) {
				if (size.type == type) 
				{
					size.descriptorCount = count;					
					return *this; // found
				}
			}
			// not found
			m_pool_sizes.push_back({ type, count });
			return *this;
		}

	private:
		uint32_t get_total_set_count() {
			uint32_t sum = 0;
			for (auto& size : m_pool_sizes) {
				sum += size.descriptorCount;
			}
			return sum;
		}
	};

	VulkanDescriptorPool(VkDevice device, VkDescriptorPoolCreateInfo& info)
		:m_vk_device(device)
	{
		if (vkCreateDescriptorPool(device, &info, nullptr, &m_descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}


};

VulkanDescriptorPool::Builder::operator VulkanDescriptorPool() {
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.poolSizeCount = m_pool_sizes.size();
	info.pPoolSizes = m_pool_sizes.data();
	info.maxSets = get_total_set_count();
	return VulkanDescriptorPool(m_vk_device, info);
}

inline void VulkanDescriptorPool::move_members(VulkanDescriptorPool&& other) {
	m_vk_device = other.m_vk_device;
	m_descriptor_pool = other.m_descriptor_pool;
	other.m_descriptor_pool = VK_NULL_HANDLE;
}

inline void VulkanDescriptorPool::free_members() {
	if (m_descriptor_pool != VK_NULL_HANDLE) {
		// When a pool is destroyed, all descriptor sets allocated from the pool are implicitly freed and become invalid.
		vkDestroyDescriptorPool(m_vk_device, m_descriptor_pool, nullptr);
		m_descriptor_pool = VK_NULL_HANDLE;
	}
}

inline VulkanDescriptorSet VulkanDescriptorPool::allocate_descriptor_set(const VulkanDescriptorSetLayout& layout)
{
	return allocate_descriptor_set(layout.m_vk_descriptor_set_layout);
}

inline void VulkanDescriptorPool::free_descriptor_set(const VulkanDescriptorSet& descriptor_set)
{
	free_descriptor_set(descriptor_set.m_vk_descriptor_set);
}

inline void VulkanDescriptorPool::free_descriptor_set(const VkDescriptorSet& vk_descriptor_set)
{
	auto result = vkFreeDescriptorSets(m_vk_device, m_descriptor_pool, 1, &vk_descriptor_set);
	ensure(result == VK_SUCCESS, "descriptor set was freed");
}

inline VulkanDescriptorSet VulkanDescriptorPool::allocate_descriptor_set(const VkDescriptorSetLayout& vk_layout)
{
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.pNext = nullptr;
	info.descriptorPool = m_descriptor_pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &vk_layout;
	return VulkanDescriptorSet(m_vk_device, info);
}
