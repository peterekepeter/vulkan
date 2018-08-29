// archeoptical.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Vulkan.hpp"
#include "Window.hpp"


void configureDefaultConsole(ApplicationServices& app) {
	app.dependency.For<Win32DefaultConsoleDriver>().UseDefaultConstructor();
	app.dependency.For<IConsoleDriver>().UseType<Win32DefaultConsoleDriver>();
	app.console.UseDriver(app.dependency.GetInstance<IConsoleDriver>());
}

int main()
{
	ApplicationServices app;
	configureDefaultConsole(app);
	app.console.Open().Output << "Starting application!\n";

	VulkanApplication vulkan(true, true);
	VulkanDebugUtilsMessenger debug(vulkan.instance, app.console);


	// the actual window
	bool running = true;
	InitWindowInfo windowInitInfo = {};
	windowInitInfo.onCloseWindow = [&]() {
		running = false;
	};
	auto hwnd = InitWindow(windowInitInfo);

	// create window for vulkan
	VulkanWindow vulkanWindow(vulkan.instance, hwnd);

	// devices
	VulkanPhysicalDeviceEnumeration physicalDevices(vulkan.instance, vulkanWindow.surface);
	if (physicalDevices.anyDevice == false) throw std::runtime_error("No device was found with Vulkan support. Check drivers!");
	if (physicalDevices.anySuitable == false) throw std::runtime_error("No device was found with all required features!");
	auto& physical = *physicalDevices.top;

	// print info about physical device
	app.console.Open().Output << "Using " << physical.properties.deviceName << " for rendering purposes...\n";

	// logical vulkan device
	VulkanDevice device(vulkan, physical);


	// process win32 message loop
	while (running) {
		ProcessWindowMessages();
	}
    return 0;
}

