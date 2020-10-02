#pragma once


class VulkanRenderPass
{
public:
	VkDevice m_vk_device;
	VkRenderPass m_vk_render_pass;

	VulkanRenderPass(VkDevice device, VkRenderPassCreateInfo& createInfo) : m_vk_device(device) {
		switch (vkCreateRenderPass(m_vk_device, &createInfo, nullptr, &m_vk_render_pass)) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkAllocateMemory: out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkAllocateMemory: out of device memory");
		default:
			throw std::runtime_error("vkAllocateMemory: failed");
		}
	}

	VulkanRenderPass(VulkanRenderPass&& other) noexcept 
		: m_vk_device(other.m_vk_device)
		, m_vk_render_pass(other.m_vk_render_pass) 
	{
		other.m_vk_render_pass = VK_NULL_HANDLE;
	}

	VulkanRenderPass(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator =(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator =(VulkanRenderPass&&) = delete;

	~VulkanRenderPass() 
	{
		if (m_vk_render_pass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_vk_device, m_vk_render_pass, nullptr);
		}
	}

	class Builder
	{
	public:
		VkDevice m_vk_device;
		VkRenderPassCreateInfo m_info;
		std::vector<VkAttachmentDescription> m_attachement_descriptions;
		std::vector<VkSubpassDescription> m_subpass_descriptions;
		std::vector<VkSubpassDependency> m_subpass_dependencies;

		Builder(VkDevice device) : m_vk_device(device) {
			m_info = {};
			m_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		}

		operator VulkanRenderPass() {
			m_info.attachmentCount = m_attachement_descriptions.size();
			m_info.pAttachments = m_attachement_descriptions.data();
			m_info.subpassCount = m_subpass_descriptions.size();
			m_info.pSubpasses = m_subpass_descriptions.data();
			m_info.dependencyCount = m_subpass_dependencies.size();
			m_info.pDependencies = m_subpass_dependencies.data();
			for (auto& subpass_builder : m_subpass_builders)
			{
				subpass_builder.build_descriptor();
			}
			return VulkanRenderPass(m_vk_device, m_info);
		}

		class Subpass;
		Builder::Subpass& add_subpass();

		class Attachment {
		public:
			Builder& m_builder;
			VkAttachmentDescription& m_description;

			Attachment(Builder& builder, VkAttachmentDescription& description) : m_builder(builder) , m_description(description) { }

			static void SetDefaults(VkAttachmentDescription& attachment_description) {
				attachment_description = {};
				attachment_description.flags = 0;
				attachment_description.format = VK_FORMAT_R8G8B8A8_SRGB;
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}

			// shortcuts

			Attachment& from(const VulkanImage& image) { return set_format(image.m_vk_format).set_samples(image.m_vk_sample_count_flag_bits); }

			Attachment& load_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachment& clear_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachment& dont_care_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachment& store_final_color_depth() { return set_store_op(VK_ATTACHMENT_STORE_OP_STORE); }
			Attachment& dont_care_final_color_depth() { return set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachment& load_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachment& clear_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachment& dont_care_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachment& store_final_stencil() { return set_stencil_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }
			Attachment& dont_care_final_stencil() { return set_stencil_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachment& color_and_depth_op(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return set_load_op(loadOp).set_store_op(storeOp); }
			Attachment& stencil_op(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return set_stencil_load_op(loadOp).set_stencil_store_op(storeOp); }
			Attachment& initial_layout_color_attachment() { return set_initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachment& final_layout_color_attachment() { return set_final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachment& final_layout_present_src() { return set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); }
			Attachment& final_layout_transfer_src() { return set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); }
			Attachment& final_layout_transfer_dst() { return set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); }

			// property setters

			Attachment& set_attachment_flags(VkAttachmentDescriptionFlags flags) { m_description.flags = flags; return *this; }
			Attachment& set_format(VkFormat format) { m_description.format = format; return *this; }
			Attachment& set_samples(VkSampleCountFlagBits samples) { m_description.samples = samples; return *this; }
			Attachment& set_load_op(VkAttachmentLoadOp loadOp) { m_description.loadOp = loadOp; return *this; }
			Attachment& set_store_op(VkAttachmentStoreOp storeOp) { m_description.storeOp = storeOp; return *this; }
			Attachment& set_stencil_load_op(VkAttachmentLoadOp stencilLoadOp) { m_description.stencilLoadOp = stencilLoadOp; return *this; }
			Attachment& set_stencil_store_op(VkAttachmentStoreOp stencilStoreOp) { m_description.stencilStoreOp = stencilStoreOp; return *this; }
			Attachment& set_initial_layout(VkImageLayout initialLayout){ m_description.initialLayout = initialLayout; return *this; }
			Attachment& set_final_layout(VkImageLayout finalLayout){ m_description.finalLayout = finalLayout; return *this; }

			// return expressions

			operator VulkanRenderPass() { return m_builder; }
			Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { return m_builder.SetRenderPassFlags(flags); }
			Builder::Attachment attachment(uint32_t index) { return m_builder.attachment(index); }
			Builder::Subpass& subpass(uint32_t index) { return m_builder.subpass(index); }
			Builder::Attachment add_attachment() { return m_builder.add_attachment(); }
			Builder::Attachment add_attachment(const VulkanImage& image) { return m_builder.add_attachment(image); }
			Builder::Subpass& add_subpass() { return m_builder.add_subpass(); }
		};

		class Subpass
		{
			friend class Builder;
		public:
			Builder& m_builder;
			VkSubpassDescription& m_description;
			std::vector<VkAttachmentReference> m_color_attachments;


			Subpass(Builder& builder, VkSubpassDescription& description)
				: m_builder(builder)
				, m_description(description)
			{
				m_description = {};
				m_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			}

			Subpass& writes_color_attachment(uint32_t index) { m_color_attachments.push_back({ index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); return *this; }
			Subpass& writes_color_attachment(uint32_t index, VkImageLayout layout) { m_color_attachments.push_back({ index, layout }); return *this; }

			operator VulkanRenderPass() { return m_builder; }
			Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { return m_builder.SetRenderPassFlags(flags); }
			Builder::Attachment attachment(uint32_t index) { return m_builder.attachment(index); }
			Builder::Subpass& subpass(uint32_t index) { return m_builder.subpass(index); }
			Builder::Attachment add_attachment() { return m_builder.add_attachment(); }
			Builder::Attachment add_attachment(const VulkanImage& image) { return m_builder.add_attachment(image); }
			Builder::Subpass& add_subpass() { return m_builder.add_subpass(); }

		private:

			// must be called by parent builder before it builds
			void build_descriptor() {
				m_description.colorAttachmentCount = m_color_attachments.size();
				m_description.pColorAttachments = m_color_attachments.data();
			}
		};

		// property setters

		Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { m_info.flags = flags; return *this; }

		// subexpressions

		Builder::Attachment add_attachment() { m_attachement_descriptions.push_back({}); Attachment::SetDefaults(m_attachement_descriptions.back()); return Attachment(*this, m_attachement_descriptions.back()); }
		Builder::Attachment add_attachment(const VulkanImage& image) { 
			auto subexpression = add_attachment();
			subexpression.from(image);
			return subexpression;
		}

		Builder::Attachment attachment(uint32_t index)
		{
			while (m_attachement_descriptions.size() <= index)
			{
				add_attachment();
			}
			return Attachment(*this, m_attachement_descriptions[index]);
		}

		Builder::Subpass& subpass(uint32_t index) {
			while (m_subpass_builders.size() <= index)
			{
				add_subpass();
			}
			return m_subpass_builders[index];
		}

		std::vector<Subpass> m_subpass_builders;


	};

};

inline VulkanRenderPass::Builder::Subpass& VulkanRenderPass::Builder::add_subpass() {
	m_subpass_descriptions.emplace_back();
	m_subpass_builders.emplace_back(*this, m_subpass_descriptions.back());
	return m_subpass_builders.back();
}