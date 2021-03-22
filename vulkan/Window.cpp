#include "stdafx.h"

#include "Window.hpp"

#define WINDOW_CLASS_NAME (L"WindowClass")

static bool did_global_init = false;

struct create_win32_window_result { HWND hwnd; int client_width; int client_height; };

static void disable_vk_steam_overlay(bool disabled);
static create_win32_window_result create_win32_window(int nCmdShow, bool fullscreen, bool borderless, int xres = 0, int yres = 0, const wchar_t* title = nullptr);
static void adjust_window_size(HWND hwnd, int& clientWidth, int& clientHeight, int xres, int yres);
static LRESULT CALLBACK class_wnd_proc_wrapper(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
static void register_window_class();


LRESULT Window::wnd_proc(UINT message, WPARAM wParam, LPARAM lParam)
{
	bool keySet = false;
	bool keyDown = false;
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		return DefWindowProc(m_hwnd, message, wParam, lParam);
	}
	break;
	case WM_SIZE:
		switch (wParam) {
		case SIZE_MAXIMIZED:

			break;
		case SIZE_MINIMIZED:

			break;
		case SIZE_RESTORED:

			break;
		}
		m_info.on_resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DPICHANGED:
	{
		auto g_dpi = HIWORD(wParam);
		// UpdateDpiDependentFontsAndResources();

		RECT* const prcNewWindow = (RECT*)lParam;
		SetWindowPos(m_hwnd,
			NULL,
			prcNewWindow->left,
			prcNewWindow->top,
			prcNewWindow->right - prcNewWindow->left,
			prcNewWindow->bottom - prcNewWindow->top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		break;
	}
	case WM_KEYUP:
		keySet = true;
	case WM_KEYDOWN: {
		if (keySet == false) keyDown = true;
		bool nextState = keyDown;
		m_info.on_key(int(wParam), keyDown);
		// on esc exit!
		if (wParam == VK_ESCAPE) {
			DestroyWindow(m_hwnd);
		}
		break;
	}
	case WM_DESTROY:
		m_info.on_close();
 		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(m_hwnd, message, wParam, lParam);
	}
	return 0;
}

HWND Window::get_hwnd()
{
	return m_hwnd;
}

Window::Window(const InitWindowInfo& info)
{
	auto title = L"Window";
	m_info = info;

	if (m_info.title.size() > 0) {
		title = m_info.title.c_str();
	}

	if (!did_global_init) {
		// Initialize global strings
		register_window_class();
		did_global_init = true;
	}
	disable_vk_steam_overlay(!info.disable_steam_overlay);

	auto result = create_win32_window(true, info.fullscreen, info.borderless, info.x_res, info.y_res, title);
	m_hwnd = result.hwnd;
	auto bring_to_top = BringWindowToTop(m_hwnd);
	assert(bring_to_top != FALSE, "bring window to top");
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	info.on_resize(result.client_width, result.client_height);
	ensure(m_hwnd != NULL, "window was created");
}

Window::Window(Window&& other)
{
	m_info = other.m_info;
	m_hwnd = other.m_hwnd;
	other.m_hwnd = NULL;
}

Window& Window::operator=(Window&& other)
{
	if (m_hwnd) {
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, NULL);
		auto result = DestroyWindow(m_hwnd);
		ensure(result == 0, "window was destroyed");
		m_hwnd = NULL;
	}
	m_info = other.m_info;
	m_hwnd = other.m_hwnd;
	other.m_hwnd = NULL;
	return *this;
}

Window::~Window()
{
	if (!m_hwnd) { return; }
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, NULL);
	auto result = DestroyWindow(m_hwnd);
	ensure(result != 0, "window was destroyed");
	m_hwnd = NULL;
}

void process_thread_message_queue_non_blocking() {
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void process_thread_message_queue() {
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static create_win32_window_result create_win32_window(int nCmdShow, bool fullscreen, bool borderless, int xres, int yres, const wchar_t* title)
{
	int clientWidth = 0, clientHeight = 0;
	HWND result_hwnd = NULL;
	HINSTANCE h_instance = GetModuleHandle(NULL);

	if (fullscreen)
	{
		if (xres == 0 || yres == 0)
		{
			xres = GetSystemMetrics(SM_CXSCREEN);
			yres = GetSystemMetrics(SM_CYSCREEN);
		}

		if (!borderless)
		{
			int i = 0; 
			DEVMODEA devmode = {};
			bool found = false;
			while (!found && EnumDisplaySettingsA(nullptr, i, &devmode) == TRUE) {
				if (devmode.dmPelsWidth == xres && devmode.dmPelsHeight == yres) {
					found = true;
				}
				i++;
			}
			
			if (found) {
				auto change_resolution_result = ChangeDisplaySettingsA(&devmode, CDS_FULLSCREEN);
				assert(change_resolution_result == DISP_CHANGE_SUCCESSFUL, "successfully changed resolution");
			}
			result_hwnd = CreateWindowW(WINDOW_CLASS_NAME, title, WS_POPUP | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0);
		}
		else 
		{
			result_hwnd = CreateWindowW(WINDOW_CLASS_NAME, title, WS_POPUP | WS_VISIBLE,
				0, 0, xres, yres, nullptr, nullptr, h_instance, nullptr);
		}
		clientWidth = xres;
		clientHeight = yres;
		ShowCursor(0);
	}
	else
	{
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		int newWidth, newHeight;
		bool unspecifiedSize = xres == 0 || yres == 0;

		if (unspecifiedSize)
		{
			newWidth = CW_USEDEFAULT;
			newHeight = CW_USEDEFAULT;
		}
		else 
		{
			RECT clientRect = { 64, 64, 64 + xres, 64 + yres };
			AdjustWindowRectEx(&clientRect, dwStyle, false, dwStyle);
			newWidth = clientRect.right - clientRect.left;
			newHeight = clientRect.bottom - clientRect.top;
		}

		result_hwnd = CreateWindowW(WINDOW_CLASS_NAME, title, dwStyle,
			CW_USEDEFAULT, 0, newWidth, newHeight,
			nullptr, nullptr, h_instance, nullptr);
		ShowWindow(result_hwnd, nCmdShow);
		SetWindowTextW(result_hwnd, title);

		RECT clientRect;
		GetClientRect(result_hwnd, &clientRect);
		clientWidth = clientRect.right - clientRect.left;
		clientHeight = clientRect.bottom - clientRect.top;

		if (!unspecifiedSize)
		{
			// adjusts clientWidth and clie ntHeight to required size
			adjust_window_size(result_hwnd, clientWidth, clientHeight, xres, yres);
		}
	}

	ensure(result_hwnd != NULL, "window was created");

	return create_win32_window_result{
		result_hwnd, clientWidth, clientHeight
	};
}

static void adjust_window_size(HWND _hwnd, int& clientWidth, int& clientHeight, int xres, int yres)
{
	int adjustmentTry = 0, maxAdjustmentTry = 3;

	while (clientWidth != xres || clientHeight != yres)
	{
		RECT clientRect;
		GetClientRect(_hwnd, &clientRect);
		clientWidth = clientRect.right - clientRect.left;
		clientHeight = clientRect.bottom - clientRect.top;

		if (adjustmentTry++ >= maxAdjustmentTry) {
			break; // stop after 3 tries
		}

		RECT windowRect;
		GetWindowRect(_hwnd, &windowRect);
		int newWidth = windowRect.right - windowRect.left + xres - clientWidth;
		int newHeight = windowRect.bottom - windowRect.top + yres - clientHeight;
		MoveWindow(_hwnd, windowRect.left, windowRect.top, newWidth, newHeight, false);
	}
}

static LRESULT CALLBACK class_wnd_proc_wrapper(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	auto ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (ptr == NULL) {
		return DefWindowProc(hwnd, message, w_param, l_param);
	}
	else {
		auto window = reinterpret_cast<Window*>(ptr);
		return window->wnd_proc(message, w_param, l_param);
	}
}

static void register_window_class()
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = class_wnd_proc_wrapper;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	wcex.hIconSm = nullptr;

	RegisterClassExW(&wcex);
}

static void disable_vk_steam_overlay(bool disabled)
{
	const wchar_t* value = disabled ? L"1" : nullptr;
	auto result = SetEnvironmentVariableW(L"DISABLE_VK_LAYER_VALVE_steam_overlay_1", value);
	ensure(result == TRUE, "steam_overlay env var was edited");
}
