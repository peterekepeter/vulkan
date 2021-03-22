#include "stdafx.h"
#include "VulkanWindow.h"

VulkanWindow::VulkanWindow(VkInstance instance, HWND hWnd) : instance(instance) {
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = hWnd;
	createInfo.hinstance = GetModuleHandle(nullptr);

	auto create_surface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	assert(create_surface != nullptr, "vulkan API exists");

	auto status = create_surface(instance, &createInfo, nullptr, &surface);
	ensure(status == VK_SUCCESS, "created vulkan surface");
}

VulkanWindow::~VulkanWindow()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}
