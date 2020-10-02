#pragma once
#include "CommonImageFormats.h"

class VulkanImage
{
public:
	VkDevice m_vk_device;
	VkImage m_vk_image;
	VulkanMemory m_memory;

	// nice to have so we can create other compatible objects
	VkImageType m_vk_image_type;
	VkFormat m_vk_format;
	VkSampleCountFlagBits m_vk_sample_count_flag_bits;

	VulkanImage() 
		: m_vk_device(VK_NULL_HANDLE)
		, m_vk_image(VK_NULL_HANDLE)
		, m_vk_image_type(VK_IMAGE_TYPE_2D)
		, m_vk_format(VK_FORMAT_R8G8B8A8_SRGB)
		, m_vk_sample_count_flag_bits(VK_SAMPLE_COUNT_1_BIT)
	{
	}

	VulkanImage(VkDevice device, const VkImageCreateInfo& createInfo) : m_vk_device(device) {
		m_vk_image_type = createInfo.imageType;
		m_vk_format = createInfo.format;
		m_vk_sample_count_flag_bits = createInfo.samples;
		switch (vkCreateImage(m_vk_device, &createInfo, nullptr, &m_vk_image)) {
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
		if (m_vk_image != VK_NULL_HANDLE) {
			vkDestroyImage(m_vk_device, m_vk_image, nullptr);
			m_vk_image = VK_NULL_HANDLE;
		}
	}

	VulkanImage(VulkanImage&& other) noexcept {
		m_vk_device = other.m_vk_device;
		m_vk_image = other.m_vk_image;
		m_vk_format = other.m_vk_format;
		m_vk_sample_count_flag_bits = other.m_vk_sample_count_flag_bits;
		m_vk_image_type = other.m_vk_image_type;
		m_memory = std::move(other.m_memory);
		other.m_vk_image = VK_NULL_HANDLE;
	}

	VulkanImage(const VulkanImage& other) = delete;
	VulkanImage& operator =(const VulkanImage& other) = delete;
	VulkanImage& operator =(VulkanImage&& other) = delete;

	class Builder
	{
	public:

		operator VulkanImage() {
			createInfo.queueFamilyIndexCount = indexes.size();
			createInfo.pQueueFamilyIndices = indexes.data();
			VulkanImage image = VulkanImage(imageMemoryAllocator.device, createInfo);
			switch (memoryTypeHostVisible) {
			case true:
				image.m_memory = imageMemoryAllocator.AllocateHostVisibleAndCoherent(image.m_vk_image);
				break;
			case false:
				image.m_memory = imageMemoryAllocator.AllocateDeviceLocal(image.m_vk_image);
				break;
			}
			imageMemoryAllocator.BindImageMemory(image.m_vk_image, image.m_memory);
			return image;
		}

		static const VkFormat DefaultImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

		VulkanImageMemoryAllocator& imageMemoryAllocator;
		VkImageCreateInfo createInfo;
		std::vector<uint32_t> indexes;
		bool memoryTypeHostVisible = false;

		Builder(VulkanImageMemoryAllocator& allocator) : imageMemoryAllocator(allocator) {
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			clear_flag_bits()
				.set_image_type(VK_IMAGE_TYPE_2D)
				.set_format(Builder::DefaultImageFormat)
				.set_extent(1, 1, 1)
				.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED)
				.set_mip_levels(1)
				.set_array_layers(1)
				.set_sample_count(VK_SAMPLE_COUNT_1_BIT)
				.optimal_tiling()
				.clear_flag_bits()
				.set_sharing_mode(VK_SHARING_MODE_EXCLUSIVE)
				.clear_queue_family_indexes();
		}

		// commonly used and supported RGBA formats

		Builder& format_R8G8B8A8_UNORM() { return set_format(CommonImageFormats::R8G8B8A8_UNORM); }
		Builder& format_R8G8B8A8_SRGB() { return set_format(CommonImageFormats::R8G8B8A8_SRGB); }
		Builder& format_R16G16B16A16_SFLOAT() { return set_format(CommonImageFormats::R16G16B16A16_SFLOAT); }
		Builder& format_R32G32B32A32_SFLOAT() { return set_format(CommonImageFormats::R32G32B32A32_SFLOAT); }

		// commonly used and supported 2 component formats

		Builder& format_R8G8_UNORM() { return set_format(CommonImageFormats::R8G8_UNORM); }
		Builder& format_R16G16_SFLOAT() { return set_format(CommonImageFormats::R16G16_SFLOAT); }
		Builder& format_R32G32_SFLOAT() { return set_format(CommonImageFormats::R32G32_SFLOAT); }

		// commonly used and supported 1 component formats

		Builder& format_R8_UNORM() { return set_format(CommonImageFormats::R8_UNORM); }
		Builder& format_R16_SFLOAT() { return set_format(CommonImageFormats::R16_SFLOAT); }
		Builder& format_R32_SFLOAT() { return set_format(CommonImageFormats::R32_SFLOAT); }

		// shortcuts

		Builder& image_1d(uint32_t width) { return set_image_type(VK_IMAGE_TYPE_1D).set_extent(width); }
		Builder& image_2d(uint32_t width, uint32_t height) { return set_image_type(VK_IMAGE_TYPE_2D).set_extent(width, height); }
		Builder& image_3d(uint32_t width, uint32_t height, uint32_t depth) { return set_image_type(VK_IMAGE_TYPE_3D).set_extent(width, height, depth); }
		Builder& image_cube(uint32_t widthAndHeight) { return image_2d(widthAndHeight, widthAndHeight).set_array_layers(6).set_flag_bits(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT); }
		Builder& host_visible_and_coherent() { return set_tiling(VK_IMAGE_TILING_LINEAR).set_memory_host_visible_and_coherent(true); }
		Builder& optimal_tiling() { return set_tiling(VK_IMAGE_TILING_OPTIMAL); }
		Builder& linear_tiling() { return set_tiling(VK_IMAGE_TILING_LINEAR); }
		Builder& share_with(uint32_t queueFamilyIndex) { return set_sharing_mode(VK_SHARING_MODE_CONCURRENT).add_queue_family_index(queueFamilyIndex); }
		Builder& usage_transfer_src() { return add_usage_flag_bits(VK_IMAGE_USAGE_TRANSFER_SRC_BIT); }
		Builder& usage_transfer_dst() { return add_usage_flag_bits(VK_IMAGE_USAGE_TRANSFER_DST_BIT); }
		Builder& usage_transfer_src_dst() { return usage_transfer_src().usage_transfer_dst(); }
		Builder& usage_sampled() { return add_usage_flag_bits(VK_IMAGE_USAGE_SAMPLED_BIT); }
		Builder& usage_storage() { return add_usage_flag_bits(VK_IMAGE_USAGE_STORAGE_BIT); }
		Builder& usage_color_attachment() { return add_usage_flag_bits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT); }
		Builder& usage_depth_stencil_attachment() { return add_usage_flag_bits(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT); }
		Builder& usage_transient_attachment() { return add_usage_flag_bits(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT); }
		Builder& usage_input_attachment() { return add_usage_flag_bits(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT); }

		// properties

		Builder& set_flag_bits(VkImageCreateFlagBits flag) { createInfo.flags = flag; return *this; }
		Builder& clear_flag_bits() { createInfo.flags = 0; return *this; }
		Builder& set_image_type(VkImageType type) { createInfo.imageType = type; return *this; }
		Builder& set_format(VkFormat format) { createInfo.format = format; return *this; }
		Builder& set_extent(VkExtent3D extent) { createInfo.extent = extent; return *this; }
		Builder& set_extent(uint32_t width, uint32_t height = 1, uint32_t depth = 1) { createInfo.extent = { width, height, depth }; return *this; }
		Builder& set_mip_levels(uint32_t count) { createInfo.mipLevels = count; return *this; };
		Builder& set_array_layers(uint32_t count) { createInfo.arrayLayers = count; return *this; };
		Builder& set_sample_count(VkSampleCountFlagBits sampleCount) { createInfo.samples = sampleCount; return *this; };
		Builder& set_tiling(VkImageTiling imageTiling) { createInfo.tiling = imageTiling; return *this; };
		Builder& set_usage_flag_bits(VkImageUsageFlagBits usage) { createInfo.usage = usage; return *this; };
		Builder& add_usage_flag_bits(VkImageUsageFlagBits usage) { createInfo.usage |= usage; return *this; };
		Builder& clear_usage_flag_bits() { createInfo.usage = 0; return *this; };
		Builder& set_sharing_mode(VkSharingMode sharingMode) { createInfo.sharingMode = sharingMode; return *this; };
		Builder& set_initial_layout(VkImageLayout layout) { createInfo.initialLayout = layout; return *this; };
		Builder& add_queue_family_index(uint32_t index) { indexes.push_back(index); return *this; };
		Builder& clear_queue_family_indexes() { indexes.clear(); return *this; };
		Builder& set_memory_host_visible_and_coherent(bool hostVisibleAndCoherent) { memoryTypeHostVisible = hostVisibleAndCoherent; return *this; }

	};

};
