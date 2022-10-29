#pragma once
#include "pch.h"

struct Configuration
{
	bool allowControls = true;
	bool vulkanValidation = true;
	bool messenger = true;
	bool liveReload = true;
	bool fullscreen = false;
	bool borderless = false;
	bool alwaysOnTop = false;
	int xres = 0;
	int yres = 0;
	int fps = 0;
	int sample_count = 1;
	int frame_count = -1;
	int device_index = -1;
	int vertices = 3;
	std::wstring windowTitle;
	std::string musicFile;
	std::string outFile;
	std::string curvesFile;
	std::unique_ptr<std::string> vulkan_sdk = nullptr;
	bool musicStream = true;
	bool musicEnabled = true;
	bool offline = false;
};

class ConfigurationBuilder
{
	int argc = 0;
	char** argv = nullptr;
	char** env = nullptr;
	const char* fileName = nullptr;
	Configuration* configuration = nullptr;
public:
	ConfigurationBuilder& UseEnvironment(char** env);
	ConfigurationBuilder& UseConsoleArgs(int argc, char** argv);
	ConfigurationBuilder& UseConfigurationFile(const char* configFile);
	Configuration* Build();
private:
	void ReadConfigurationFile();
	void ReadConsoleConfig();
	void ReadEnv();
};