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
			return VulkanRenderPass(m_vk_device, m_info);
		}

		class Subpass;
		Builder::Subpass& add_subpass();

		class Attachement {
		public:
			Builder& m_builder;
			VkAttachmentDescription& m_description;

			Attachement(Builder& builder, VkAttachmentDescription& description) : m_builder(builder) , m_description(description) { }

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

			Attachement& load_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachement& clear_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachement& dont_care_initial_color_depth() { return set_load_op(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachement& store_final_color_depth() { return set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }
			Attachement& dont_care_final_color_depth() { return set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachement& load_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachement& clear_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachement& dont_care_initial_stencil() { return set_stencil_load_op(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachement& store_final_stencil() { return set_stencil_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }
			Attachement& dont_care_final_stencil() { return set_stencil_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachement& color_and_depth_op(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return set_load_op(loadOp).set_store_op(storeOp); }
			Attachement& stencil_op(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return set_stencil_load_op(loadOp).set_stencil_store_op(storeOp); }
			Attachement& initial_layout_color_attachment() { return set_initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachement& final_layout_color_attachment() { return set_final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachement& final_layout_present_src() { return set_final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); }
			Attachement& final_layout_transfer_src() { return set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); }
			Attachement& final_layout_transfer_dst() { return set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); }

			// property setters

			Attachement& set_attachment_flags(VkAttachmentDescriptionFlags flags) { m_description.flags = flags; return *this; }
			Attachement& set_format(VkFormat format) { m_description.format = format; return *this; }
			Attachement& set_samples(VkSampleCountFlagBits samples) { m_description.samples = samples; return *this; }
			Attachement& set_load_op(VkAttachmentLoadOp loadOp) { m_description.loadOp = loadOp; return *this; }
			Attachement& set_store_op(VkAttachmentStoreOp storeOp) { m_description.storeOp = storeOp; return *this; }
			Attachement& set_stencil_load_op(VkAttachmentLoadOp stencilLoadOp) { m_description.stencilLoadOp = stencilLoadOp; return *this; }
			Attachement& set_stencil_store_op(VkAttachmentStoreOp stencilStoreOp) { m_description.stencilStoreOp = stencilStoreOp; return *this; }
			Attachement& set_initial_layout(VkImageLayout initialLayout){ m_description.initialLayout = initialLayout; return *this; }
			Attachement& set_final_layout(VkImageLayout finalLayout){ m_description.finalLayout = finalLayout; return *this; }

			// return expressions

			operator VulkanRenderPass() { return m_builder; }
			Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { return m_builder.SetRenderPassFlags(flags); }
			Builder::Attachement add_attachment() { return m_builder.add_attachment(); }
			Builder::Attachement add_attachment(const VulkanImage& image) { return m_builder.add_attachment(image); }
			Builder::Subpass& add_subpass() { return m_builder.add_subpass(); }
		};

		class Subpass
		{
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

			Subpass& ref_color_attachment(uint32_t index) { m_color_attachments.push_back({ index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); return *this; }
			Subpass& ref_color_attachment(uint32_t index, VkImageLayout layout) { m_color_attachments.push_back({ index, layout }); return *this; }

			operator VulkanRenderPass() { return m_builder; }
			Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { return m_builder.SetRenderPassFlags(flags); }
			Builder::Attachement add_attachment() { return m_builder.add_attachment(); }
			Builder::Attachement add_attachment(const VulkanImage& image) { return m_builder.add_attachment(image); }
			Builder::Subpass& add_subpass() { return m_builder.add_subpass(); }
		};

		// property setters

		Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { m_info.flags = flags; return *this; }

		// subexpressions

		Builder::Attachement add_attachment() { m_attachement_descriptions.push_back({}); Attachement::SetDefaults(m_attachement_descriptions.back()); return Attachement(*this, m_attachement_descriptions.back()); }
		Builder::Attachement add_attachment(const VulkanImage& image) { 
			auto subexpression = add_attachment();
			subexpression.set_format(image.vkFormat).set_samples(image.vkSampleCountFlagBits);
			return subexpression;
		}

		std::vector<Subpass> m_subpass_builders;


	};

};

inline VulkanRenderPass::Builder::Subpass& VulkanRenderPass::Builder::add_subpass() {
	m_subpass_descriptions.emplace_back();
	m_subpass_builders.emplace_back(*this, m_subpass_descriptions.back());
	return m_subpass_builders.back();
}