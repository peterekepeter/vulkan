#pragma once

#include "stdafx.h"

class VulkanDebugUtilsMessenger {
public:

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	VkDebugUtilsMessengerEXT messenger;
	Console& console;
	VkInstance& instance;


	VulkanDebugUtilsMessenger(VkInstance& instance, Console& console);
	~VulkanDebugUtilsMessenger();

private:

	static const char* SeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity);

	static const char* TypeToString(VkDebugUtilsMessageTypeFlagsEXT type);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

};

class VulkanApplication
{
public:
	VkInstance instance;
	VkApplicationInfo appInfo;
	VkInstanceCreateInfo createInfo;
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;
	std::vector<const char*> requiredLayers;
	std::vector<const char*> requiredExtensions;

	VulkanApplication(bool enableValidation = false, bool windowSupport = false);

	VulkanApplication(const VulkanApplication&) = delete;
	VulkanApplication& operator =(const VulkanApplication&) = delete;
	VulkanApplication(VulkanApplication&&) = delete;
	VulkanApplication& operator =(VulkanApplication&& other) = delete;

	~VulkanApplication();
};

void printAvaiableExtensions(Console& console, const VulkanApplication& vulkan);

class VulkanPhysicalDevice
{
public:
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<VkQueueFamilyProperties> queueFamilies;
	int score = 0;
	bool suitable = false;
	int graphicsFamilyIndex = -1;
	int computeFamilyIndex = -1;
	int presentFamilyIndex = -1;

	VulkanPhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	VulkanPhysicalDevice(const VulkanPhysicalDevice& other) = delete;
	VulkanPhysicalDevice& operator = (const VulkanPhysicalDevice& other) = delete;
	VulkanPhysicalDevice(VulkanPhysicalDevice&& other);
	VulkanPhysicalDevice& operator = (VulkanPhysicalDevice&& other);

private:

	int calculateScore();

	bool isSuitable();

};

class VulkanPhysicalDeviceEnumeration {
public:
	std::vector<VulkanPhysicalDevice> physicalDevices;
	VulkanPhysicalDevice* top = nullptr;
	bool anySuitable = false;
	bool anyDevice = false;

	VulkanPhysicalDeviceEnumeration(VkInstance instance, VkSurfaceKHR surface);
};


class VulkanDevice
{
public:
	VkDeviceQueueCreateInfo queueCreateInfo;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkDeviceCreateInfo createInfo;
	VkDevice device;
	VkQueue graphicsQueue;

	VulkanDevice(VulkanApplication& vulkan, VulkanPhysicalDevice& physicalDevice);

	~VulkanDevice();
};
