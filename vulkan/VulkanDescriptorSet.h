#pragma once

class VulkanDescriptorSet
{
public:
	VkDevice m_vk_device;
	VkDescriptorSet m_vk_descriptor_set;

	VulkanDescriptorSet() 
		: m_vk_device(nullptr)
		, m_vk_descriptor_set(0) 
	{
	
	}

	void update_to_uniform_buffer(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) {
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = buffer;
		buffer_info.offset = offset;
		buffer_info.range = range;
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_vk_descriptor_set;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &buffer_info;
		write.pImageInfo = nullptr; // Optional
		write.pTexelBufferView = nullptr; // Optional
		vkUpdateDescriptorSets(m_vk_device, 1, &write, 0, nullptr);
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