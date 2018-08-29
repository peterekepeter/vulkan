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
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<const char*> requiredExtensions;
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkQueueFamilyProperties> queueFamilies;
	int score;
	bool suitable;
	bool swapChainAdequate;
	bool extensionsSupported;
	int graphicsFamilyIndex = -1;
	int computeFamilyIndex = -1;
	int presentFamilyIndex = -1;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	void init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

private:

	void querySwapChainSupport();
	void findAndCheckExtensions();
	void findQueues();

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
	std::vector<const char*> requiredExtensions;
	VkDeviceCreateInfo createInfo;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue presentQueue;

	VulkanDevice(VulkanApplication& vulkan, VulkanPhysicalDevice& physicalDevice);

	~VulkanDevice();
};

class VulkanSwapChain {
public:
	VkSwapchainKHR swapChain;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	VkDevice logicalDevice;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VulkanSwapChain(const VulkanPhysicalDevice& physicalDevice, const VulkanDevice& device, int defaultWidth, int defaultHeight);
	~VulkanSwapChain();
};