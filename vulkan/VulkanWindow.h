#pragma once

class VulkanWindow {
public:
	VkInstance instance;
	VkSurfaceKHR surface;

	VulkanWindow(VkInstance instance, HWND hwnd);

	~VulkanWindow();
};