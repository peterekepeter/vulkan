#include "stdafx.h"
#include "VulkanApplicationBuilder.h"

using names = std::vector<const char*>;
using str = std::string;

VulkanApplicationBuilder::VulkanApplicationBuilder(Console& c) : console(c)
{
	memset(&info, 0, sizeof(VkApplicationInfo));
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pNext = nullptr;
	info.apiVersion = VK_API_VERSION_1_0;
	info.pApplicationName = "N/A";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "N/A";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	logger = nullptr;
}

struct VulkanExtensionEnumeration
{
	VulkanExtensionEnumeration() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		extensions.resize(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	}

	names ResolveExtensions(const names& required, const names& preferred, Console& console) {
		names result;
		for (auto& name : required) {
			if (!HasExtension(name)) {
				throw std::runtime_error(str("Required extension ") + name + " not found!");
			}
			else {
				result.push_back(name);
			}
		}
		for (auto& name : preferred) {
			if (!HasExtension(name)) {
				console.Open().Error << str("Optional extension ") + name + "not found...\n";
			} else {
				result.push_back(name);
			}
		}
		return result;
	}

private:

	std::vector<VkExtensionProperties> extensions;

	bool HasExtension(const char* required) {
		// check required extensions
		for (auto& props : extensions) {
			if (std::strcmp(required, props.extensionName) == 0) {
				return true;
			}
		}
		return false;
	}
};

struct VulkanLayerEnumeration
{

	VulkanLayerEnumeration() {
		// enumerate vk layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		layers.resize(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
	}

	names ResolveLayers(const names& required, const names& preferred, Console& console) {
		names result;
		for (auto& name : required) {
			if (!HasLayer(name)) {
				throw std::runtime_error(str("Required layer ") + name + " not found!");
			}
			else {
				result.push_back(name);
			}
		}
		for (auto& name : preferred) {
			if (!HasLayer(name)) {
				console.Open().Error << str("Optional layer ") + name + " not found...\n";
			}
			else {
				result.push_back(name);
			}
		}
		return result;
	}

private:

	std::vector<VkLayerProperties> layers;

	bool HasLayer(const char* required) {
		// check required extensions
		for (auto& props : layers) {
			if (std::strcmp(required, props.layerName) == 0) {
				return true;
			}
		}
		return false;
	}
};

VkInstanceCreateInfo prepare_create_info(VkApplicationInfo &info, names& enabled_extensions, names& enabled_layers) {
	VkInstanceCreateInfo createInfo;
	// configure vk create info
	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
	createInfo.ppEnabledExtensionNames = enabled_extensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
	createInfo.ppEnabledLayerNames = enabled_layers.data();
	return createInfo;
}

VulkanApplication VulkanApplicationBuilder::Build()
{
	auto enabled_extensions = VulkanExtensionEnumeration().ResolveExtensions(requiredDeviceExtensions, preferredDeviceExtensions, console);
	auto enabled_layers = VulkanLayerEnumeration().ResolveLayers(requiredLayers, preferredLayers, console);

	auto result = VulkanApplication(
		prepare_create_info(info, enabled_extensions, enabled_layers),
		enabled_layers);

	if (logger != nullptr)
	{
		if (VulkanDebugUtilsMessenger::IsSupportedByInstance(result.instance)) {
			result.debugMessenger = std::make_unique<VulkanDebugUtilsMessenger>(
				result.instance,
				std::move(logger));
		} else {
			console.Open().Error << "Vulkan Debugging Utilities are not supported, debug information not available! (SDK not properly installed)\n";
		}
	}
	return result;
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
VulkanApplicationBuilder::EnableValidationLayer(
	bool enabled)
{
	if (!enabled) {
		return *this;
	}
	preferredLayers.push_back("VK_LAYER_KHRONOS_validation");
	return *this;
}

VulkanApplicationBuilder& VulkanApplicationBuilder::UseLogger(
	std::function<void(
		VkDebugUtilsMessageSeverityFlagBitsEXT, 
		VkDebugUtilsMessageTypeFlagsEXT, 
		const VkDebugUtilsMessengerCallbackDataEXT*)> logger)
{
	this->logger = logger;
	// add debug utils if loggers was added
	if (this->logger != nullptr) {
		preferredDeviceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return *this;
}

VulkanApplicationBuilder& 
VulkanApplicationBuilder::EnableWindowSupport(
	bool enabled)
{
	if (!enabled) {
		return *this;
	}
	requiredDeviceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredDeviceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	return *this;
}

VulkanApplicationBuilder::operator VulkanApplication()
{
	return Build();
}
