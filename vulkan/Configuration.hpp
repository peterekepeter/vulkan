#pragma once
#include "stdafx.h"

struct Configuration
{
	bool allowControls = true;
	bool vulkanValidation = true;
	bool messenger = true;
	bool liveReload = true;
	bool fullscreen = false;
	bool borderless = false;
	int xres = 0;
	int yres = 0;
	int fps = 60;
	std::wstring windowTitle;
	std::string musicFile;
	bool musicStream = true;
	bool musicEnabled = true;
	bool offline = false;
};

class ConfigurationBuilder
{
	int argc = 0;
	char** argv = nullptr;
	const char* fileName = nullptr;
	Configuration* configuration = nullptr;
	ApplicationServices* app = nullptr;
public:
	ConfigurationBuilder& UseConsoleArgs(int argc, char** argv);
	ConfigurationBuilder& UseConfigurationFile(const char* configFile);
	Configuration* Build(ApplicationServices& app);
private:
	void ReadConfigurationFile();
	void ReadConsoleConfig();
};