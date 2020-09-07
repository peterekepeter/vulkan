#pragma once


class VulkanRenderPass
{
public:
	VkDevice vkDeviceHandle;
	VkRenderPass vkRenderPassHandle;

	VulkanRenderPass(VkDevice device, VkRenderPassCreateInfo& createInfo) : vkDeviceHandle(device) {
		switch (vkCreateRenderPass(vkDeviceHandle, &createInfo, nullptr, &vkRenderPassHandle)) {
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
		: vkDeviceHandle(other.vkDeviceHandle)
		, vkRenderPassHandle(other.vkRenderPassHandle) 
	{
		other.vkRenderPassHandle = VK_NULL_HANDLE;
	}

	VulkanRenderPass(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator =(const VulkanRenderPass&) = delete;
	VulkanRenderPass& operator =(VulkanRenderPass&&) = delete;

	~VulkanRenderPass() 
	{
		if (vkRenderPassHandle != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(vkDeviceHandle, vkRenderPassHandle, nullptr);
		}
	}

	class Builder
	{
	public:
		VkDevice vkDeviceHandle;
		VkRenderPassCreateInfo createInfo;
		std::vector<VkAttachmentDescription> attachementDescriptions;
		std::vector<VkSubpassDescription> subpassDescriptions;
		std::vector<VkSubpassDependency> subpassDependencies;

		Builder(VkDevice device) : vkDeviceHandle(device) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		}

		operator VulkanRenderPass() {
			createInfo.attachmentCount = attachementDescriptions.size();
			createInfo.pAttachments = attachementDescriptions.data();
			createInfo.subpassCount = subpassDescriptions.size();
			createInfo.pSubpasses = subpassDescriptions.data();
			createInfo.dependencyCount = subpassDependencies.size();
			createInfo.pDependencies = subpassDependencies.data();
			return VulkanRenderPass(vkDeviceHandle, createInfo);
		}

		class Attachement {
		public:
			Builder& builder;
			VkAttachmentDescription& description;

			Attachement(Builder& builder, VkAttachmentDescription& description) : builder(builder) , description(description) { }

			static void SetDefaults(VkAttachmentDescription& description) {
				description = {};
				description.flags = 0;
				description.format = VK_FORMAT_R8G8B8A8_SRGB;
				description.samples = VK_SAMPLE_COUNT_1_BIT;
				description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				description.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}

			// shortcuts

			Attachement& LoadInitialColorDepth() { return SetLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachement& ClearInitialColorDepth() { return SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachement& DontCareInitialColorDepth() { return SetLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachement& StoreFinalColorDepth() { return SetStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }
			Attachement& DontCareFinalColorDepth() { return SetStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachement& LoadInitialStencil() { return SetStencilLoadOp(VK_ATTACHMENT_LOAD_OP_LOAD); }
			Attachement& ClearInitialStencil() { return SetStencilLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR); }
			Attachement& DontCareInitialStencil() { return SetStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE); }
			Attachement& StoreFinalStencil() { return SetStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }
			Attachement& DontCareFinalStencil() { return SetStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE); }

			Attachement& ColorAndDepthOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return SetLoadOp(loadOp).SetStoreOp(storeOp); }
			Attachement& StencilOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp) { return SetStencilLoadOp(loadOp).SetStencilStoreOp(storeOp); }
			Attachement& InitialLayoutColorAttachment() { return SetInitialLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachement& FinalLayoutColorAttachment() { return SetFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); }
			Attachement& FinalLayoutPresentSrc() { return SetFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); }
			Attachement& FinalLayoutTransferSrc() { return SetFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL); }
			Attachement& FinalLayoutTransferDst() { return SetFinalLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); }

			// property setters

			Attachement& SetAttachmentFlags(VkAttachmentDescriptionFlags flags) { description.flags = flags; return *this; }
			Attachement& SetFormat(VkFormat format) { description.format = format; return *this; }
			Attachement& SetSamples(VkSampleCountFlagBits samples) { description.samples = samples; return *this; }
			Attachement& SetLoadOp(VkAttachmentLoadOp loadOp) { description.loadOp = loadOp; return *this; }
			Attachement& SetStoreOp(VkAttachmentStoreOp storeOp) { description.storeOp = storeOp; return *this; }
			Attachement& SetStencilLoadOp(VkAttachmentLoadOp stencilLoadOp) { description.stencilLoadOp = stencilLoadOp; return *this; }
			Attachement& SetStencilStoreOp(VkAttachmentStoreOp stencilStoreOp) { description.stencilStoreOp = stencilStoreOp; return *this; }
			Attachement& SetInitialLayout(VkImageLayout initialLayout){ description.initialLayout = initialLayout; return *this; }
			Attachement& SetFinalLayout(VkImageLayout finalLayout){ description.finalLayout = finalLayout; return *this; }

			// return expressions

			operator VulkanRenderPass() { return builder; }
			Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { return builder.SetRenderPassFlags(flags); }
			Builder::Attachement AddAttachment() { return builder.AddAttachment(); }
			Builder::Attachement AddAttachment(const VulkanImage& image) { return builder.AddAttachment(image); }
		};

		// property setters

		Builder& SetRenderPassFlags(VkRenderPassCreateFlags flags) { createInfo.flags = flags; return *this; }

		// subexpressions

		Builder::Attachement AddAttachment() { attachementDescriptions.push_back({}); Attachement::SetDefaults(attachementDescriptions.back()); return Attachement(*this, attachementDescriptions.back()); }
		Builder::Attachement AddAttachment(const VulkanImage& image) { 
			auto subexpression = AddAttachment();
			subexpression.SetFormat(image.vkFormat).SetSamples(image.vkSampleCountFlagBits);
			return subexpression;
		}

	};

};
