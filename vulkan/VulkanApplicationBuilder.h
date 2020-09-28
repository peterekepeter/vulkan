#pragma once
#include "Vulkan.hpp"


class VulkanApplicationBuilder
{
public:
	VulkanApplicationBuilder(Console& console);
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
	Console& console;
	VkApplicationInfo info;
	std::vector<const char*> preferredDeviceExtensions;
	std::vector<const char*> preferredLayers;
	std::vector<const char*> requiredDeviceExtensions;
	std::vector<const char*> requiredLayers;
	std::function<void(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT, const
		VkDebugUtilsMessengerCallbackDataEXT*)> logger;
};