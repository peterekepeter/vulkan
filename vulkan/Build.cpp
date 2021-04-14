#include "pch.h"
#include "Build.h"
#include "Configuration.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/AutoBuild.hpp"

void make_sure_dir_exists(const std::string dirPath)
{
	if (CreateDirectoryA(dirPath.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return; /// ok
	}
	else
	{
		throw std::runtime_error("failed to create dir");
	}
}

bool rebuild_shaders(ApplicationServices& app)
{
	auto& config = *app.dependency.GetInstance<Configuration>();
	auto& console = *app.dependency.GetInstance<Console>();
	auto& atb = *app.dependency.GetInstance<AutoBuild>();

	if (config.vulkan_sdk == nullptr) {
		throw std::runtime_error("rebuilding shaders requires Vulkan SDK!\n"
			"Download from: https://vulkan.lunarg.com/sdk/home#windows");
	}
	console.Open().Output << "Rebuilding shaders\n";
	bool success = true;

	std::string toolsPath = (*config.vulkan_sdk) + "/Bin";
	std::string compiler = toolsPath + "/glslangValidator.exe";
	std::string optimizer = toolsPath + "/spirv-opt.exe";

	atb.tool("glsl", [compiler, &console](const char* in, const char* out) {
		auto cmd = compiler + " -V " + in + " -o " + out;
		auto result = ProcessCommand::Execute(cmd);
		if (result.exitCode != 0) {
			console.Open().Error << result.output;
		}
		return result.exitCode;
		});

	atb.tool("spv-opt", [optimizer, &console](const char* in, const char* out) {
		auto cmd = optimizer + " -O " + in + " -o " + out;
		auto result = ProcessCommand::Execute(cmd);
		if (result.exitCode != 0) {
			console.Open().Error << result.output;
		}
		return result.exitCode;
		});

	make_sure_dir_exists("build");
	atb.step("glsl", "shader.frag", "build/shader.frag.spv");
	atb.step("glsl", "shader.vert", "build/shader.vert.spv");
	atb.step("spv-opt", "build/shader.frag.spv", "build/shader.frag.opt.spv");
	atb.step("spv-opt", "build/shader.vert.spv", "build/shader.vert.opt.spv");
	atb.wait_idle();
	return success;
}
