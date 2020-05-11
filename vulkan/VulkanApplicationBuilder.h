#pragma once
#include "Vulkan.hpp"


class VulkanApplicationBuilder
{
public:
	VulkanApplicationBuilder();
	VulkanApplication Build();
	VulkanApplicationBuilder& ApiVersion(uint32_t version); // default VK_API_VERSION_1_0
	VulkanApplicationBuilder& ApplicationInfo(const char* name, uint32_t version);
	VulkanApplicationBuilder& EngineInfo(const char* name, uint32_t version);
	VulkanApplicationBuilder& RequireValidationLayer(bool enabled = true);
	VulkanApplicationBuilder& RequireWindowSupport(bool enabled = true);
private:
	VkApplicationInfo info;
	std::vector<const char*> requiredExtensions;
	std::vector<const char*> requiredLayers;
};