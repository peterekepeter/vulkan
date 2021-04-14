#include "pch.h"
#include "./VulkanDebugUtilsMessenger.h"


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessenger::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	void* pUserData) {

	auto self = (VulkanDebugUtilsMessenger*)pUserData;
	self->logger(messageSeverity, messageType, pCallbackData);
	return VK_FALSE; 
}

VulkanDebugUtilsMessenger::VulkanDebugUtilsMessenger(
	VkInstance instance,
	std::function<void(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*)> logger)
	: instance(instance)
	, logger(logger)
{	// setup create info
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity
		= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		;
	createInfo.messageType 
		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = this;

	messenger = {};
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func == nullptr)
		throw std::runtime_error("vkCreateDebugUtilsMessengerEXT not found");
	if (func(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS)
		throw std::runtime_error("vkCreateDebugUtilsMessengerEXT failed"); 
}

VulkanDebugUtilsMessenger::~VulkanDebugUtilsMessenger() {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, messenger, nullptr);
	}
}

bool VulkanDebugUtilsMessenger::IsSupportedByInstance(VkInstance instance)
{
	return vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") != nullptr;
}
