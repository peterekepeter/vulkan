// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "framework.h"

// windows
#define NOMINMAX
#include <Windows.h>

// C libs
#include <stdio.h>
#include <tchar.h>

// C++ standard lib
#include <algorithm>
#include <chrono>
#include <set>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <locale>
#include <codecvt>

// vulkan things
#include <vulkan\vulkan.h>
#include <vulkan\vulkan_win32.h>

// app service sandwich
#include "../submodule/app-service-sandwich/AppServiceSandwich/ApplicationServices.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/Win32DefaultConsoleDriver.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/DirectoryChangeService.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/ProcessCommand.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/Assertions.h"

// audio
#include "../submodule/simple-audio/simple-audio/Music.h"

// reference additional headers your program requires here

#define DECLARE_MOVEABLE_TYPE(T) \
private: \
	void move_members(T&&); \
	void free_members(); \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t){ move_members(std::move(t)); }; \
	T& operator =(T&& t){ free_members(); move_members(std::move(t)); return *this; }; \
	~T() { free_members(); }; 

#define DECLARE_MOVEABLE_COPYABLE_TYPE(T) \
private: \
	void copy_members(const T&); \
	void move_members(T&&); \
	void free_members(); \
public: \
	T(const T& t) { copy_members(t); } \
	T& operator =(const T& t) { free_members(); copy_members(t); return *this; } \
	T(T&& t){ move_members(std::move(t)); }; \
	T& operator =(T&& t){ free_members(); move_members(std::move(t)); return *this; }\
	~T() { free_members(); }

#define DECLARE_DEFAULT_MOVEABLE_TYPE(T) \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t) = default; \
	T& operator =(T&& t) = default; 

#define DECLARE_DEFAULT_MOVEABLE_COPYABLE_TYPE(T) \
public: \
	T(const T&) = default; \
	T& operator =(const T&) = default; \
	T(T&& t) = default; \
	T& operator =(T&& t) = default; 

#define DECLARE_NOT_MOVEABLE_OR_COPYABLE_TYPE(T) \
public: \
	T(const T&) = delete; \
	T& operator =(const T&) = delete; \
	T(T&& t) = delete; \
	T& operator =(T&& t) = delete; 