#include "stdafx.h"
#include "Configuration.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/CommandLineParser.hpp"

Configuration* ConfigurationBuilder::Build(ApplicationServices& app)
{
	if (this->configuration != nullptr) {
		throw std::logic_error("a ConfigurationBuilder can only build once");
	}
	this->app = &app;
	this->configuration = new Configuration();
	if (this->fileName != nullptr) {
		ReadConfigurationFile();
	}
	if (this->argc != 0) {
		ReadConsoleConfig();
	}
	return this->configuration;
}

ConfigurationBuilder& ConfigurationBuilder::UseConsoleArgs(int argc, char** argv)
{
	this->argc = argc;
	this->argv = argv;
	return *this;
}

ConfigurationBuilder& ConfigurationBuilder::UseConfigurationFile(const char* configFile)
{
	this->fileName = configFile;
	return *this;
}

void ConfigurationBuilder::ReadConsoleConfig()
{
	ApplicationServices& app = *this->app;
	Configuration& config = *this->configuration;

	bool borderless = false;
	bool windowed = false;

	CommandLineParser()
		.Option("-b", "--borderless", borderless)
		.Option("-w", "--windowed", windowed)
		.Option("-x", "--xres", config.xres)
		.Option("-y", "--yres", config.yres)
		.Option("-d", "--device-index", config.device_index)
		.Parse(argv, argc);

	config.fullscreen = !windowed;
	config.borderless = borderless;
}

// this function will skip the UTF-8 BOM header in the given stream if detected at current position
static void skipBomHeader(std::ifstream& file) {
	// detect BOM header https://en.wikipedia.org/wiki/Byte_order_mark
	union header { char bytes[4]; uint32_t word; };
	constexpr header detect = { 0x01020304 };
	static_assert(sizeof(header) == 4, "header union must have 4 bytes.");
	constexpr bool little_endian = detect.bytes[0] == 0x04;
	constexpr uint32_t mask = little_endian ? 0x00ffffff : 0xffffff00;
	constexpr uint32_t utf8 = little_endian ? 0x00bfbbef : 0xefbbbf00;
	// read first 4 bytes from file
	header buffer;
	file.read(buffer.bytes, 4);
	std::streamoff offset = (mask & buffer.word) == utf8 ? -1 : -4;
	file.seekg(offset, std::ios_base::cur);
}

void ConfigurationBuilder::ReadConfigurationFile()
{
	ApplicationServices& app = *this->app;
	Configuration& config = *this->configuration;
	auto configFilePath = "config.ini";

	std::ifstream infile(configFilePath);
	if (!infile.good()) {
		app.console.Open().Error << "Configuration file " << configFilePath << " not found!\n";
		throw std::exception("Failed to read configuration file.");
	}

	skipBomHeader(infile);

	app.console.Open().Output << "Reading configurationfile " << configFilePath << "\n";
	std::string line;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	config.musicEnabled = false;
	while (std::getline(infile, line))
	{
		auto separator_pos = line.find_first_of('=');
		if (separator_pos == std::string::npos) {
			continue;
		}
		std::string key = line.substr(0, separator_pos);
		std::string value = line.substr(separator_pos + 1);
		if (key == "title") {
			config.windowTitle = converter.from_bytes(value);
		}
		else if (key == "audio") {
			config.musicFile = value;
			config.musicEnabled = true;
		}
		else if (key == "stream") {
			config.musicStream = value == "1";
		}
		else if (key == "liveReload") {
			config.liveReload = value == "1";
		}
	}
}