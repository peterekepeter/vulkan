#pragma once




class VulkanImage
{
public:
	VkDevice vkDeviceHandle;
	VkImage vkImageHandle;
	VulkanMemory memory;

	VulkanImage() : vkDeviceHandle(VK_NULL_HANDLE), vkImageHandle(VK_NULL_HANDLE) { }

	VulkanImage(VkDevice device, const VkImageCreateInfo& createInfo) : vkDeviceHandle(device) {
		switch (vkCreateImage(vkDeviceHandle, &createInfo, nullptr, &vkImageHandle)) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkCreateImage: failed, ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkCreateImage: failed, ran out of device memory");
		default:
			throw std::runtime_error("vkCreateImage: failed");
		}
	}

	~VulkanImage() {
		if (vkImageHandle != VK_NULL_HANDLE) {
			vkDestroyImage(vkDeviceHandle, vkImageHandle, nullptr);
			vkImageHandle = VK_NULL_HANDLE;
		}
	}

	VulkanImage(VulkanImage&& other) noexcept {
		vkDeviceHandle = other.vkDeviceHandle;
		vkImageHandle = other.vkImageHandle;
		memory = std::move(other.memory);
		other.vkImageHandle = VK_NULL_HANDLE;
	}

	VulkanImage(const VulkanImage& other) = delete;
	VulkanImage& operator =(const VulkanImage& other) = delete;
	VulkanImage& operator =(VulkanImage&& other) = delete;

	class Builder
	{
	public:

		operator VulkanImage() {
			createInfo.queueFamilyIndexCount = indices.size();
			createInfo.pQueueFamilyIndices = indices.data();
			VulkanImage image = VulkanImage(imageMemoryAllocator.device, createInfo);
			switch (memoryTypeHostVisible) {
			case true:
				image.memory = imageMemoryAllocator.AllocateHostVisibleAndCoherent(image.vkImageHandle);
			case false:
				image.memory = imageMemoryAllocator.AllocateDeviceLocal(image.vkImageHandle);
			}
			return image;
		}

		static const VkFormat DefaultImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

		VulkanImageMemoryAllocator& imageMemoryAllocator;
		VkImageCreateInfo createInfo;
		std::vector<uint32_t> indices;
		bool memoryTypeHostVisible = false;

		Builder(VulkanImageMemoryAllocator& allocator) : imageMemoryAllocator(allocator) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ClearFlagBits()
				.SetImageType(VK_IMAGE_TYPE_2D)
				.SetFormat(Builder::DefaultImageFormat)
				.SetExtent(1, 1, 1)
				.SetInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.SetMipLevels(1)
				.SetArrayLayers(1)
				.SetSampleCount(VK_SAMPLE_COUNT_1_BIT)
				.OptimalTiling()
				.ClearFlagBits()
				.SetSharingMode(VK_SHARING_MODE_EXCLUSIVE)
				.ClearQueueFamilyIndices();
		}

		// shortcuts

		Builder& HostVisibleAndCoherent() { return SetTiling(VK_IMAGE_TILING_LINEAR).SetMemoryType(true); }
		Builder& OptimalTiling() { return SetTiling(VK_IMAGE_TILING_OPTIMAL); }
		Builder& LinearTiling() { return SetTiling(VK_IMAGE_TILING_LINEAR); }
		Builder& ShareWith(uint32_t queueFamilyIndex) { return SetSharingMode(VK_SHARING_MODE_CONCURRENT).AddQueueFamilyIndex(queueFamilyIndex); }
		Builder& UsageTransferSrc() { return AddUsageFlagBits(VK_IMAGE_USAGE_TRANSFER_SRC_BIT); }
		Builder& UsageTransferDst() { return AddUsageFlagBits(VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
		Builder& UsageTransferSrcDst() { return UsageTransferSrc().UsageTransferDst(); }
		Builder& UsageSampled() { return AddUsageFlagBits(VK_IMAGE_USAGE_SAMPLED_BIT); }
		Builder& UsageStorage() { return AddUsageFlagBits(VK_IMAGE_USAGE_STORAGE_BIT); }
		Builder& UsageColorAttachement() { return AddUsageFlagBits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT); }
		Builder& UsageDepthStencilAttachement() { return AddUsageFlagBits(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT); }
		Builder& UsageTransientAttachement() { return AddUsageFlagBits(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT); }
		Builder& UsageInputAttachement() { return AddUsageFlagBits(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT); }

		// properties

		Builder& SetFlagBits(VkImageCreateFlagBits flag) { createInfo.flags = flag; return *this; }
		Builder& ClearFlagBits() { createInfo.flags = 0; return *this; }
		Builder& SetImageType(VkImageType type) { createInfo.imageType = type; return *this; }
		Builder& SetFormat(VkFormat format) { createInfo.format = format; return *this; }
		Builder& SetExtent(VkExtent3D extent) { createInfo.extent = extent; return *this; }
		Builder& SetExtent(uint32_t width, uint32_t height = 1, uint32_t depth = 1) { createInfo.extent = { width, height, depth }; return *this; }
		Builder& SetMipLevels(uint32_t count) { createInfo.mipLevels = count; return *this; };
		Builder& SetArrayLayers(uint32_t count) { createInfo.arrayLayers = count; return *this; };
		Builder& SetSampleCount(VkSampleCountFlagBits sampleCount) { createInfo.samples = sampleCount; return *this; };
		Builder& SetTiling(VkImageTiling imageTiling) { createInfo.tiling = imageTiling; return *this; };
		Builder& SetUsageFlagBits(VkImageUsageFlagBits usage) { createInfo.usage = usage; return *this; };
		Builder& AddUsageFlagBits(VkImageUsageFlagBits usage) { createInfo.usage |= usage; return *this; };
		Builder& ClearUsageFlagBits() { createInfo.usage = 0; return *this; };
		Builder& SetSharingMode(VkSharingMode sharingMode) { createInfo.sharingMode = sharingMode; return *this; };
		Builder& SetInitialLayout(VkImageLayout layout) { createInfo.initialLayout = layout; return *this; };
		Builder& AddQueueFamilyIndex(uint32_t index) { indices.push_back(index); return *this; };
		Builder& ClearQueueFamilyIndices() { indices.clear(); return *this; };
		Builder& SetMemoryType(bool hostVisibleAndCoherent) { memoryTypeHostVisible = hostVisibleAndCoherent; return *this; }

	};


};
