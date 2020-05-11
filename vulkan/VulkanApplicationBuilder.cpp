#include "stdafx.h"
#include "VulkanApplicationBuilder.h"

VulkanApplicationBuilder::VulkanApplicationBuilder()
{
	memset(&info, 0, sizeof(VkApplicationInfo));
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pNext = nullptr;
	info.apiVersion = VK_API_VERSION_1_0;
	info.pApplicationName = "N/A";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "N/A";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
}

VulkanApplication VulkanApplicationBuilder::Build()
{
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;

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

	// check required layers
	for (auto& required : requiredLayers) {
		bool found = false;
		for (auto& avaiable : availableLayers)
		{
			if (std::strcmp(required, avaiable.layerName) == 0)
			{
				found = true;
			}
		}
		if (found == false)
		{
			throw std::runtime_error(std::string("Required layer ") + required + " not found!");
		}
	}

	// check required extensions
	for (auto& required : requiredExtensions) {
		bool found = false;
		for (auto& avaiable : availableExtensions) {
			if (std::strcmp(required, avaiable.extensionName) == 0) {
				found = true;
			}
		}
		if (found == false) {
			throw std::runtime_error(std::string("Required extension ") + required + " not found!");
		}
	}

	VkInstanceCreateInfo createInfo;

	// configure vk create info
	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
	createInfo.ppEnabledLayerNames = requiredLayers.data();

	return VulkanApplication(createInfo, requiredLayers);
}

VulkanApplicationBuilder& VulkanApplicationBuilder::ApiVersion(
	uint32_t version)
{
	info.apiVersion = version;
	return *this;
}

VulkanApplicationBuilder& VulkanApplicationBuilder::ApplicationInfo(const char* name, uint32_t version)
{
	info.pApplicationName = name;
	info.applicationVersion = version;
	return *this;
}

VulkanApplicationBuilder& VulkanApplicationBuilder::EngineInfo(const char* name, uint32_t version)
{
	info.pEngineName = name;
	info.engineVersion = version;
	return *this;
}

VulkanApplicationBuilder& 
VulkanApplicationBuilder::RequireValidationLayer(
	bool enabled)
{
	if (!enabled) {
		return *this;
	}
	requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	return *this;
}

VulkanApplicationBuilder& 
VulkanApplicationBuilder::RequireWindowSupport(
	bool enabled)
{
	if (!enabled) {
		return *this;
	}
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	return *this;
}
