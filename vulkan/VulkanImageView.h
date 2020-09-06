#pragma once

class VulkanImageView
{
public:
	VkDevice device;
	VkImageView vkImageViewHandle;

	VulkanImageView(VkDevice device, const VkImageViewCreateInfo& createInfo) : device(device) {
		switch (vkCreateImageView(device, &createInfo, nullptr, &vkImageViewHandle)) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			throw std::runtime_error("vkCreateImageView: failed, ran out of host memory");
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			throw std::runtime_error("vkCreateImageView: failed, ran out of device memory");
		default:
			throw std::runtime_error("vkCreateImageView: failed");
		}
	}

	VulkanImageView() {
		device = VK_NULL_HANDLE;
		vkImageViewHandle = VK_NULL_HANDLE;
	}

	VulkanImageView(VulkanImageView&& other) noexcept {
		device = other.device;
		vkImageViewHandle = other.vkImageViewHandle;
		other.vkImageViewHandle = VK_NULL_HANDLE;
	}

	VulkanImageView& operator =(VulkanImageView&& other) {
		if (vkImageViewHandle != VK_NULL_HANDLE) {
			vkDestroyImageView(device, vkImageViewHandle, nullptr);
		}
		device = other.device;
		vkImageViewHandle = other.vkImageViewHandle;
		other.vkImageViewHandle = VK_NULL_HANDLE;
	};

	VulkanImageView(const VulkanImageView& other) = delete;
	VulkanImageView& operator =(const VulkanImageView& other) = delete;

	~VulkanImageView() {
		if (vkImageViewHandle != VK_NULL_HANDLE) {
			vkDestroyImageView(device, vkImageViewHandle, nullptr);
		}
	}

	class Builder
	{
	public:
		VkDevice vkDeviceHandle;
		VkImageViewCreateInfo createInfo;

		operator VulkanImageView() {
			return VulkanImageView(vkDeviceHandle, createInfo);
		}

		Builder(const VulkanImage& image) : vkDeviceHandle(image.vkDeviceHandle) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			SetDefaults();
			// we can propagate image, format and image type from a VulkanImage
			SetImage(image.vkImageHandle);
			SetFormat(image.vkFormat);
			switch (image.vkImageType) {
			case VK_IMAGE_TYPE_1D:
				SetImageViewType(VK_IMAGE_VIEW_TYPE_1D);
				break;
			case VK_IMAGE_TYPE_2D:
				SetImageViewType(VK_IMAGE_VIEW_TYPE_2D);
				break;
			case VK_IMAGE_TYPE_3D:
				SetImageViewType(VK_IMAGE_VIEW_TYPE_3D);
				break;
			default:
				throw std::runtime_error("VulkanImageView::Builder: type not implemented");
			}
		}

		Builder(VkDevice device) : vkDeviceHandle(device) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			SetDefaults();
			SetImageViewType(VK_IMAGE_VIEW_TYPE_2D)
				.SetFormat(VulkanImage::Builder::DefaultImageFormat);
		}

		Builder& SetFlags(VkImageViewCreateFlags flags) { this->createInfo.flags = flags; return *this; }
		Builder& SetImage(VkImage image) { this->createInfo.image = image; return *this; }
		Builder& SetImageViewType(VkImageViewType viewType) { this->createInfo.viewType = viewType; return *this; }
		Builder& SetFormat(VkFormat format) { this->createInfo.format = format; return *this; }
		Builder& SetComponentMapping(const VkComponentMapping& components) { this->createInfo.components = components; return *this; }
		Builder& SetSubresourceRange(const VkImageSubresourceRange& subresourceRange) { this->createInfo.subresourceRange = subresourceRange; return *this; }
		Builder& SetSubresourceRangeAspectMask(VkImageAspectFlags flags) { this->createInfo.subresourceRange.aspectMask = flags; return *this; }
		Builder& SetSubresourceRangeBaseMipLevel(uint32_t base) { this->createInfo.subresourceRange.baseMipLevel = base; return *this; }
		Builder& SetSubresourceRangeMipLevelCount(uint32_t count) { this->createInfo.subresourceRange.levelCount = count; return *this; }
		Builder& SetSubresourceRangeBaseArrayLayer(uint32_t base) { this->createInfo.subresourceRange.baseArrayLayer = base; return *this; }
		Builder& SetSubresourceRangeArrayLayerCount(uint32_t count) { this->createInfo.subresourceRange.layerCount = count; return *this; }
	private:

		void SetDefaults() {
			SetComponentMapping(VkComponentMapping{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY })
				.SetSubresourceRangeAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
				.SetSubresourceRangeBaseMipLevel(0)
				.SetSubresourceRangeMipLevelCount(VK_REMAINING_MIP_LEVELS)
				.SetSubresourceRangeBaseArrayLayer(0)
				.SetSubresourceRangeArrayLayerCount(VK_REMAINING_ARRAY_LAYERS);
		}
	};
};
