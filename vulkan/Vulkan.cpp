#include "stdafx.h"
#include "Vulkan.hpp"

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
	resetSwapChain();
}

void VulkanPhysicalDevice::resetSwapChain()
{
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

	auto canBeSource = capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

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
		requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	// check that we have all required extensions
	extensionsSupported = true;
	for (auto& required : requiredDeviceExtensions) {
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
	return 
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

VulkanApplication::VulkanApplication(
	const VkInstanceCreateInfo& createInfo,
	const std::vector<const char*>& requiredLayers) {
	this->requiredLayers = requiredLayers;
	auto vkResult = vkCreateInstance(&createInfo, nullptr, &instance);
	if (vkResult != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance.");
}

VulkanApplication::VulkanApplication(VulkanApplication&& other) noexcept
	: instance(other.instance)
	, requiredLayers(std::move(other.requiredLayers))
	, debugMessenger(std::move(other.debugMessenger))
{
	other.instance = nullptr;
}

VulkanApplication& VulkanApplication::operator=(VulkanApplication&& other) noexcept
{
	instance = other.instance;
	other.instance = nullptr;
	requiredLayers = std::move(other.requiredLayers);
	debugMessenger = std::move(other.debugMessenger);
	return *this;
}

VulkanApplication::~VulkanApplication()
{
	if (instance == nullptr) return;
	// cleanup
	debugMessenger = nullptr; // kill the messenger first
	vkDestroyInstance(instance, nullptr);
	instance = nullptr;
}