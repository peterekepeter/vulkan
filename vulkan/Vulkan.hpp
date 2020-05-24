#pragma once
#include "VulkanDebugUtilsMessenger.h"
#include "VulkanShader.h"

class VulkanApplication
{
public:
	VkInstance instance; 
	std::vector<const char*> requiredLayers;

	VulkanApplication(const VkInstanceCreateInfo& createInfo,
		const std::vector<const char*>& requiredLayers);

	VulkanApplication(const VulkanApplication&) = delete;
	VulkanApplication& operator =(const VulkanApplication&) = delete;
	VulkanApplication(VulkanApplication&&) noexcept;
	VulkanApplication& operator =(VulkanApplication&& other) noexcept;

	~VulkanApplication();

	std::unique_ptr<VulkanDebugUtilsMessenger> debugMessenger;
};

class VulkanPhysicalDevice
{
public:
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<const char*> requiredDeviceExtensions;
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
	void resetSwapChain();

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
	std::vector<const char*> requiredDeviceExtensions;
	VkDeviceCreateInfo createInfo;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue computeQueue;
	VkQueue presentQueue;

	VulkanDevice(VulkanApplication& vulkan, VulkanPhysicalDevice& physicalDevice);
	VulkanShader CreateShader(const std::vector<char> binary);
	VulkanShader CreateShader(const char* binary, const size_t size);

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