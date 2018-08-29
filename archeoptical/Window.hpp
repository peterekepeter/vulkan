#pragma once

#include "stdafx.h"

class InitWindowInfo {
public:
	// todo add stuff
	std::function<void()> onCloseWindow = nullptr;
};

HWND InitWindow(const InitWindowInfo& info);
void ProcessWindowMessages();

class VulkanWindow {
public:
	VkInstance instance;
	VkSurfaceKHR surface;

	VulkanWindow(VkInstance instance, HWND hwnd);
	
	~VulkanWindow();
};

void ApplyEnvVarChanges();