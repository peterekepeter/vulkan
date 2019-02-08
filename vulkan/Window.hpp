#pragma once

#include "stdafx.h"

class InitWindowInfo {
public:
	// todo add stuff
	std::function<void()> onCloseWindow = nullptr;
	std::function<void(int, int)> onWindowResize = nullptr;
	std::function<void(int, bool)> onKeystateChange = nullptr;
	std::function<void()> onWindowPaint = nullptr;
	bool fullscreen = false;
	bool borderless = false;
	int xres = 0;
	int yres = 0;
	std::wstring title;
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