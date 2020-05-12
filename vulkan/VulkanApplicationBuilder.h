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
	VulkanApplicationBuilder& EnableValidationLayer(bool enabled = true);
	VulkanApplicationBuilder& UseLogger(
		std::function<void(
			VkDebugUtilsMessageSeverityFlagBitsEXT,
			VkDebugUtilsMessageTypeFlagsEXT, const
			VkDebugUtilsMessengerCallbackDataEXT*)>);
	VulkanApplicationBuilder& EnableWindowSupport(
bool enabled = true);
	operator VulkanApplication();
private:
	VkApplicationInfo info;
	std::vector<const char*> requiredExtensions;
	std::vector<const char*> requiredLayers;
	std::function<void(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT, const
		VkDebugUtilsMessengerCallbackDataEXT*)> logger;
};