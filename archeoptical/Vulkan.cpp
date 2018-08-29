
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

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) : physicalDevice(physicalDevice) {
	// get properties and featuer
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

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

	suitable = isSuitable();
	score = calculateScore();
}

VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanPhysicalDevice && other)
{
	operator=(std::move(other));
}


VulkanPhysicalDevice& VulkanPhysicalDevice::operator = (VulkanPhysicalDevice&& other) {
	physicalDevice = other.physicalDevice;
	properties = other.properties;
	features = other.features;
	queueFamilies = std::move(other.queueFamilies);
	score = other.score;
	suitable = other.suitable;
	graphicsFamilyIndex = other.graphicsFamilyIndex;
	computeFamilyIndex = other.computeFamilyIndex;
	presentFamilyIndex = other.presentFamilyIndex;
	return *this;
};

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
		graphicsFamilyIndex != -1 && presentFamilyIndex != -1 &&
		features.geometryShader;
}

VulkanPhysicalDeviceEnumeration::VulkanPhysicalDeviceEnumeration(VkInstance instance, VkSurfaceKHR surface) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> vkPysicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, vkPysicalDevices.data());

	// transform all vk devices to VulkanDevice
	for (auto& vkPhysicalDevice : vkPysicalDevices) {
		physicalDevices.emplace_back(vkPhysicalDevice, surface);
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
	queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = physicalDevice.graphicsFamilyIndex;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// setup required features
	deviceFeatures = {};

	// setup logical device creation
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.enabledLayerCount = static_cast<uint32_t>(vulkan.requiredLayers.size());
	createInfo.ppEnabledLayerNames = vulkan.requiredLayers.data();
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;

	if (vkCreateDevice(physicalDevice.physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(device, physicalDevice.computeFamilyIndex, 0, &graphicsQueue);
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
		std::ostringstream* target = &con.Error;
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
