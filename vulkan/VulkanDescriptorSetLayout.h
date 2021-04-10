#pragma once

class VulkanDescriptorSetLayout
{
	DECLARE_MOVEABLE_TYPE(VulkanDescriptorSetLayout);
public:
	VkDevice m_vk_device = nullptr;
	VkDescriptorSetLayout m_vk_descriptor_set_layout = VK_NULL_HANDLE;

	VulkanDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo& info)
		: m_vk_device(device) 
	{
		auto result = vkCreateDescriptorSetLayout(device, &info, nullptr, &m_vk_descriptor_set_layout);
		ensure(result == VK_SUCCESS, "descriptor set layour was created");
	}

	class Builder
	{
	public:
		VkDevice m_vk_device;
		bool m_is_push_descriptor;
		std::vector<VkDescriptorSetLayoutBinding> m_bindings;

		class DescriptorBuilder
		{
		public:
			Builder& m_parent;
			VkDescriptorSetLayoutBinding& m_descriptor;

			DescriptorBuilder(Builder& parent, VkDescriptorSetLayoutBinding& descriptor)
				: m_parent(parent)
				, m_descriptor(descriptor)
			{

			}

			DescriptorBuilder& of_type(VkDescriptorType type) { m_descriptor.descriptorType = type; return *this; }
			DescriptorBuilder& with_binding(uint32_t binding) { m_descriptor.binding = binding; return *this; }
			DescriptorBuilder& with_count(uint32_t count) { m_descriptor.descriptorCount = count; return *this; }
			DescriptorBuilder& with_shader_stage_flags(VkShaderStageFlags stageFlags) { m_descriptor.stageFlags = stageFlags; return *this; }
			DescriptorBuilder& with_immutable_samplers(VkSampler* pImmutableSamplers) { m_descriptor.pImmutableSamplers = pImmutableSamplers; return *this; }
			DescriptorBuilder& with_vertex_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT; return *this; }
			DescriptorBuilder& with_both_vertex_fragment_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; return *this; }
			DescriptorBuilder& with_tesselation_control_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; return *this; }
			DescriptorBuilder& with_tesselation_evaluation_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; return *this; }
			DescriptorBuilder& with_geometry_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT; return *this; }
			DescriptorBuilder& with_fragment_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT; return *this; }
			DescriptorBuilder& with_compute_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_COMPUTE_BIT; return *this; }
			DescriptorBuilder& with_all_stage_access() { m_descriptor.stageFlags |= VK_SHADER_STAGE_ALL; return *this; }

			// link to parent
			Builder& descriptor_set() { return m_parent; }
			operator VulkanDescriptorSetLayout() { return m_parent; }
			DescriptorBuilder add_sampler(uint32_t binding) { return m_parent.add_sampler(binding); }
			DescriptorBuilder add_combined_image_sampler(uint32_t binding) { return m_parent.add_combined_image_access(binding); }
			DescriptorBuilder add_sampled_image(uint32_t binding) { return m_parent.add_sampled_image(binding); }
			DescriptorBuilder add_storage_image(uint32_t binding) { return m_parent.add_storage_image(binding); }
			DescriptorBuilder add_uniform_texel_buffer(uint32_t binding) { return m_parent.add_uniform_texel_buffer(binding); }
			DescriptorBuilder add_storage_texel_buffer(uint32_t binding) { return m_parent.add_storage_texel_buffer(binding); }
			DescriptorBuilder add_uniform_buffer(uint32_t binding) { return m_parent.add_uniform_buffer(binding); }
			DescriptorBuilder add_storage_buffer(uint32_t binding) { return m_parent.add_storage_buffer(binding); }
			DescriptorBuilder add_uniform_buffer_dynamic(uint32_t binding) { return m_parent.add_uniform_buffer_dynamic(binding); }
			DescriptorBuilder add_storage_buffer_dynamic(uint32_t binding) { return m_parent.add_storage_buffer_dynamic(binding); }
			DescriptorBuilder add_input_attachment(uint32_t binding) { return m_parent.add_input_attachment(binding); }
			DescriptorBuilder add_descriptor_set_layout(VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLER, uint32_t binding = 0) { return m_parent.add_descriptor_set_layout(type, binding); }
		};

		DescriptorBuilder add_sampler(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_SAMPLER, binding); }
		DescriptorBuilder add_combined_image_access(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, binding); }
		DescriptorBuilder add_sampled_image(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding); }
		DescriptorBuilder add_storage_image(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, binding); }
		DescriptorBuilder add_uniform_texel_buffer(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, binding); }
		DescriptorBuilder add_storage_texel_buffer(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, binding); }
		DescriptorBuilder add_uniform_buffer(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding); }
		DescriptorBuilder add_storage_buffer(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding); }
		DescriptorBuilder add_uniform_buffer_dynamic(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, binding); }
		DescriptorBuilder add_storage_buffer_dynamic(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, binding); }
		DescriptorBuilder add_input_attachment(uint32_t binding) { return add_descriptor_set_layout(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, binding); }

		DescriptorBuilder add_descriptor_set_layout(VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLER, uint32_t binding = 0) {
			m_bindings.push_back({});
			auto builder = DescriptorBuilder(*this, get_last());
			builder.with_binding(binding).of_type(type).with_count(1);
			return builder;
		}

		Builder& push_descriptor() { return set_is_push_descriptor(true); }
		Builder& set_is_push_descriptor(bool is_push_descriptor) {
			this->m_is_push_descriptor = is_push_descriptor;
			return*this;
		}

		operator VulkanDescriptorSetLayout() {
			expect(m_vk_device != nullptr);
			VkDescriptorSetLayoutCreateInfo m_info = {};
			m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			m_info.bindingCount = m_bindings.size();
			m_info.pBindings = m_bindings.data();
			if (m_is_push_descriptor) {
				m_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
			}
			assert(m_info.bindingCount == m_bindings.size());
			auto result = VulkanDescriptorSetLayout(m_vk_device, m_info);
			ensure(result.m_vk_device != nullptr);
			ensure(result.m_vk_descriptor_set_layout != VK_NULL_HANDLE);
			return result;
		}

		Builder(VkDevice d)
			: m_vk_device(d)
			, m_is_push_descriptor(false)
		{}

	private:
		VkDescriptorSetLayoutBinding& get_last() { return m_bindings[m_bindings.size() - 1]; }
	};
};


void VulkanDescriptorSetLayout::move_members(VulkanDescriptorSetLayout&& from) {
	m_vk_device = from.m_vk_device;
	m_vk_descriptor_set_layout = from.m_vk_descriptor_set_layout;
	from.m_vk_descriptor_set_layout = VK_NULL_HANDLE;
}

void VulkanDescriptorSetLayout::free_members() {
	if (m_vk_descriptor_set_layout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(m_vk_device, m_vk_descriptor_set_layout, nullptr);
		m_vk_descriptor_set_layout = VK_NULL_HANDLE;
	}
}