#pragma once

#include "stdafx.h"

class InitWindowInfo {
public:
	// todo add stuff
	std::function<void()> on_close = nullptr;
	std::function<void(int, int)> on_resize = nullptr;
	std::function<void(int, bool)> on_key = nullptr;
	std::function<void()> on_paint = nullptr;
	bool fullscreen = false;
	bool borderless = false;
	bool disable_steam_overlay = true;
	int x_res = 0;
	int y_res = 0;
	std::wstring title;
};

void process_thread_message_queue();
void process_thread_message_queue_non_blocking();

class Window {

	HWND m_hwnd;
	InitWindowInfo m_info;

	DECLARE_MOVEABLE_TYPE(Window)

public:
	Window(const InitWindowInfo& info);

	LRESULT wnd_proc(UINT, WPARAM, LPARAM);
	HWND get_hwnd();
};