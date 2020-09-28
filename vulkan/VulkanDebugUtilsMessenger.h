#pragma once

class VulkanDebugUtilsMessenger {
public:
	VkDebugUtilsMessengerEXT messenger;
	VkInstance instance;

	VulkanDebugUtilsMessenger(
		VkInstance instance,
		std::function<void(
			VkDebugUtilsMessageSeverityFlagBitsEXT,
			VkDebugUtilsMessageTypeFlagsEXT, const
			VkDebugUtilsMessengerCallbackDataEXT*)> logger);

	VulkanDebugUtilsMessenger(const VulkanDebugUtilsMessenger& other) = delete;
	VulkanDebugUtilsMessenger(VulkanDebugUtilsMessenger&& other) = delete;
	VulkanDebugUtilsMessenger& operator=(const VulkanDebugUtilsMessenger& other) = delete;
	VulkanDebugUtilsMessenger& operator=(VulkanDebugUtilsMessenger&& other) = delete;

	~VulkanDebugUtilsMessenger();

	static bool IsSupportedByInstance(VkInstance instance);

private:
	std::function<void(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT, const
		VkDebugUtilsMessengerCallbackDataEXT*)> logger;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

};