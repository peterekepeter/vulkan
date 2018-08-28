// archeoptical.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class VulkanDebugUtilsMessenger {
public:

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	VkDebugUtilsMessengerEXT messenger;
	Console& console;
	VkInstance& instance;

	static const char* SeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
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

	static const char* TypeToString(VkDebugUtilsMessageTypeFlagsEXT type) {
		switch (type)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: return "Validation: ";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance: ";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: return "";
		default: return "";
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		auto self = (VulkanDebugUtilsMessenger*) pUserData;
		auto con = self->console.Open();
		std::ostringstream* target = &con.Output;
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::ostringstream* target = &con.Error;
		}
		(*target) << "Vulkan: " << TypeToString(messageSeverity) << SeverityToString(messageSeverity);
		(*target) << pCallbackData->pMessage << "\n";

		return VK_FALSE;
	}

	VulkanDebugUtilsMessenger(VkInstance& instance, Console& console) : console(console), instance(instance) {
		// setup create info
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
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
	~VulkanDebugUtilsMessenger() {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, messenger, nullptr);
		}
	}
};

void Setup() {
}

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

	VulkanApplication(bool enableValidation = false) {
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
		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		createInfo.enabledLayerCount = requiredLayers.size();
		createInfo.ppEnabledLayerNames = requiredLayers.data();

		// get vk instance 
		auto vkResult = vkCreateInstance(&createInfo, nullptr, &instance);
		if (vkResult != VK_SUCCESS) throw std::runtime_error("Failed to create Vulkan instance.");
	}

	VulkanApplication(const VulkanApplication&) = delete;
	VulkanApplication& operator =(const VulkanApplication&) = delete;
	VulkanApplication(VulkanApplication&&) = delete;
	VulkanApplication& operator =(VulkanApplication&& other) = delete;

	~VulkanApplication()
	{
		if (instance == nullptr) return;
		// cleanup
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
};

void configureDefaultConsole(ApplicationServices& app) {
	app.dependency.For<Win32DefaultConsoleDriver>().UseDefaultConstructor();
	app.dependency.For<IConsoleDriver>().UseType<Win32DefaultConsoleDriver>();
	app.console.UseDriver(app.dependency.GetInstance<IConsoleDriver>());
}

void printAvaiableExtensions(Console& console, const VulkanApplication& vulkan) {

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

int main()
{
	ApplicationServices app;
	configureDefaultConsole(app);
	app.console.Open().Output << "Starting application!\n";

	VulkanApplication vulkan(true);
	VulkanDebugUtilsMessenger debug(vulkan.instance, app.console);

	// printAvaiableExtensions(app.console, vulkan);

    return 0;
}

