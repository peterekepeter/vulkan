#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

// Vulkan support on Windows starts with Win7, makes no sense to go below that, but we should try to be compatible with it
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <SDKDDKVer.h>
