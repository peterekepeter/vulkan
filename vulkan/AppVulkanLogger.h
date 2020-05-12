#pragma once

// sends vulkan logs to ApplicationServices
class AppVulkanLogger
{
public:
	ApplicationServices& app;

	void operator()(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* data);

private:
	const char* SeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
	const char* TypeToString(VkDebugUtilsMessageTypeFlagsEXT type);
};
