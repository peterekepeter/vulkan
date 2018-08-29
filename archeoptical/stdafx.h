// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// windows
#include <Windows.h>

// C libs
#include <stdio.h>
#include <tchar.h>

// C++ standard lib
#include <algorithm>

// vulkan things
#include <vulkan\vulkan.h>
#include <vulkan\vulkan_win32.h>

// app service sandwich
#include "../submodule/app-service-sandwich/AppServiceSandwich/ApplicationServices.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/Win32DefaultConsoleDriver.hpp"

// reference additional headers your program requires here