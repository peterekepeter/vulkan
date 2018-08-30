#pragma once

#include "stdafx.h"

class InitWindowInfo {
public:
	// todo add stuff
	std::function<void()> onCloseWindow = nullptr;
	std::function<void(int, int)> onWindowResize = nullptr;
};

HWND InitWindow(const InitWindowInfo& info);
void ProcessWindowMessages();
void ProcessWindowMessagesNonBlocking();

class VulkanWindow {
public:
	VkInstance instance;
	VkSurfaceKHR surface;

	VulkanWindow(VkInstance instance, HWND hwnd);
	
	~VulkanWindow();
};

void ApplyEnvVarChanges();