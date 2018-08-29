
#include "stdafx.h"

#include "stdafx.h"

#include "Vulkan.hpp"

void printAvaiableExtensions(Console & console, const VulkanApplication & vulkan) {

	auto con = console.Open();
	con.Output << "Available extensions:\n";
	for (const auto& extension : vulkan.availableExtensions) {
		con.Output << "\t" << extension.extensionName << "\n";
	}
	con.Output << "Available layers:\n";
	for (const auto& extension : vulkan.availableLayers) {
		con.Output << "\t" << extension.layerName << " - " << extension.description << "\n";
	}
}

void VulkanPhysicalDevice::init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

	this->surface = surface;
	this->physicalDevice = physicalDevice;

	// initial values
	suitable = false;
	swapChainAdequate = false;
	graphicsFamilyIndex = -1;
	computeFamilyIndex = -1;
	presentFamilyIndex = -1;

	// get properties and featuer
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

	findAndCheckExtensions();
	findQueues();
	if (extensionsSupported)
	{
		querySwapChainSupport();
	}

	suitable = isSuitable();
	score = calculateScore();
}


void VulkanPhysicalDevice::querySwapChainSupport()
{
	// get capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	// get supported formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	}

	// get supported presentation mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	}

	swapChainAdequate = !formats.empty() && !presentModes.empty();
}

void VulkanPhysicalDevice::findAndCheckExtensions()
{
	// get extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	availableExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	// set up required device extensions
	bool requireSwapchain = true;
	if (requireSwapchain) {
		requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	// check that we have all required extensions
	extensionsSupported = true;
	for (auto& required : requiredExtensions) {
		bool found = false;
		for (auto& extension : availableExtensions) {
			if (strcmp(extension.extensionName, required) == 0) {
				found = true;
				break;
			}
		}
		if (found == false) {
			extensionsSupported = false;
			break;
		}
	}
}

void VulkanPhysicalDevice::findQueues()
{
	// get device queues
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// find graphics queue
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsFamilyIndex = i;
			break;
		}
	}
	// find compute queue
	i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			computeFamilyIndex = i;
			break;
		}
	}
	// find present queue
	i = 0;
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			presentFamilyIndex = i;
			break;
		}
	}
}

int VulkanPhysicalDevice::calculateScore() {
	int score = 0;

	if (suitable) score += 1000;

	// based on device type
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1600;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) score += 1400;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 400;

	// pretty much mandatory
	if (graphicsFamilyIndex != -1) score += 1000;
	if (computeFamilyIndex != -1) score += 1000;

	return score;
}

bool VulkanPhysicalDevice::isSuitable() {
	return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		extensionsSupported == true &&
		swapChainAdequate == true &&
		graphicsFamilyIndex != -1 && 
		presentFamilyIndex != -1 &&
		features.geometryShader;
}

VulkanPhysicalDeviceEnumeration::VulkanPhysicalDeviceEnumeration(VkInstance instance, VkSurfaceKHR surface) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> vkPysicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, vkPysicalDevices.data());

	// transform all vk devices to VulkanDevice
	physicalDevices.resize(deviceCount);
	int i = 0;
	for (auto& physicalDevice : physicalDevices) {
		physicalDevice.init(vkPysicalDevices[i], surface);
		i++;
	}

	// aggregate, find top score and if we have any suitable
	int maxScore = -1;
	for (auto& physicalDevice : physicalDevices) {
		if (physicalDevice.score > maxScore) {
			maxScore = physicalDevice.score;
			top = &physicalDevice;
		}
		anySuitable |= physicalDevice.suitable;
		anyDevice = true;
	}
}

VulkanDevice::VulkanDevice(VulkanApplication & vulkan, VulkanPhysicalDevice& physicalDevice)
{
	// setup queue create info
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { 
		physicalDevice.graphicsFamilyIndex, 
		physicalDevice.computeFamilyIndex, 
		physicalDevice.presentFamilyIndex
	};
	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// setup required features
	deviceFeatures = {};

	// setup require extensions
	requiredExtensions = physicalDevice.requiredExtensions;

	// setup logical device creation
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan.requiredLayers.size());
	createInfo.ppEnabledLayerNames = vulkan.requiredLayers.data();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.enabledExtensionCount = requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if (vkCreateDevice(physicalDevice.physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(device, physicalDevice.graphicsFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, physicalDevice.computeFamilyIndex, 0, &computeQueue);
	vkGetDeviceQueue(device, physicalDevice.presentFamilyIndex, 0, &presentQueue);
}

VulkanDevice::~VulkanDevice() {
	vkDestroyDevice(device, nullptr);
}

VulkanApplication::VulkanApplication(bool enableValidation, bool windowSupport) {
	// configure vk application info
	memset(&appInfo, 0, sizeof(VkApplicationInfo));
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pApplicationName = "archeoptical";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	// enumerate vk extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	availableExtensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	// enumerate vk layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	availableLayers.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// require layers
	if (enableValidation) {
		requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	// requried extension if we want to open window
	if (windowSupport) {
		requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	}

	// check required layers
	for (auto& required : requiredLayers) {
		bool found = false;
		for (auto &avaiable : availableLayers) if (std::strcmp(required, avaiable.layerName) == 0) found = true;
		if (found == false) throw std::runtime_error(std::string("Required layer ") + required + " not found!");
	}

	// check required extensions
	for (auto& required : requiredExtensions) {
		bool found = false;
		for (auto &avaiable : availableExtensions) if (std::strcmp(required, avaiable.extensionName) == 0) found = true;
		if (found == false) throw std::runtime_error(std::string("Required extension ") + required + " not found!");
	}

	// configure vk create info
	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	createInfo.ppEnabledLayerNames = requiredLayers.data();

	// get vk instance 
	auto vkResult = vkCreateInstance(&createInfo, nullptr, &instance);
	if (vkResult != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance.");
}

VulkanApplication::~VulkanApplication()
{
	if (instance == nullptr) return;
	// cleanup
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
}

const char * VulkanDebugUtilsMessenger::SeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Debug: ";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: return "Info: ";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warn: ";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: return "Fail: ";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: return "";
	default:
		return "";
	}
}

const char * VulkanDebugUtilsMessenger::TypeToString(VkDebugUtilsMessageTypeFlagsEXT type) {
	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: return "Validation: ";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance: ";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: return "";
	default: return "";
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilsMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData) {

	auto self = (VulkanDebugUtilsMessenger*)pUserData;
	auto con = self->console.Open();
	std::ostringstream* target = &con.Output;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		target = &con.Error;
	}
	(*target) << "Vulkan: " << TypeToString(messageSeverity) << SeverityToString(messageSeverity);
	(*target) << pCallbackData->pMessage << "\n";

	return VK_FALSE;
}

VulkanDebugUtilsMessenger::VulkanDebugUtilsMessenger(VkInstance & instance, Console & console) : console(console), instance(instance) {
	// setup create info
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = this; // Optional

								 // attach callback
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

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VulkanSwapChain::VulkanSwapChain(const VulkanPhysicalDevice& physicalDevice, const VulkanDevice& device, int width, int height)
{
	logicalDevice = device.device;
	surfaceFormat = chooseSwapSurfaceFormat(physicalDevice.formats);
	presentMode = chooseSwapPresentMode(physicalDevice.presentModes);
	extent = chooseSwapExtent(physicalDevice.capabilities, width, height);

	uint32_t imageCount = physicalDevice.capabilities.minImageCount + 1;
	if (physicalDevice.capabilities.maxImageCount > 0 && imageCount > physicalDevice.capabilities.maxImageCount) {
		imageCount = physicalDevice.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = physicalDevice.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	// VK_IMAGE_USAGE_TRANSFER_DST_BIT postprocessing

	uint32_t queueFamilyIndices[] = { (uint32_t)physicalDevice.graphicsFamilyIndex, (uint32_t)physicalDevice.presentFamilyIndex };

	if (physicalDevice.graphicsFamilyIndex != physicalDevice.presentFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// stuff
	createInfo.preTransform = physicalDevice.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	// for resizeable windows this will change
	createInfo.oldSwapchain = VK_NULL_HANDLE; 
	
	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	// create views

	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}

}

VulkanSwapChain::~VulkanSwapChain()
{
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
}
