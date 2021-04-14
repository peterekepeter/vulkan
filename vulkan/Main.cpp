#include "pch.h"
#include "ApplicationMain.h"
#include "Configuration.hpp"
#include "../submodule/app-service-sandwich/AppServiceSandwich/AutoBuild.hpp"
#include "Playback.hpp"

static void configure_application(ApplicationServices& app, int argc, char** argv, char** env);

int main(int argc, char** argv, char** env)
{
	// some win32 function calls depend on this being defined
	expect(UNICODE == 1 && _UNICODE == 1, "compiled with unicode flag")

		ApplicationServices app;
	int exit_code = 0;
	bool first_run = true;

	try
	{
		RunResult result;
		while (first_run || result.requested_reload)
		{
			first_run = false;
			app.console.Open().Output << "Configuring application!\n";
			configure_application(app, argc, argv, env);
			app.console.Open().Output << "Starting application!\n";
			result = run_application(app);
		}
	}
	catch (const char* message) {
		app.console.Open().Error << "Fail: " << message << "\n";
		exit_code = 1;
	}
	catch (std::exception exception) {
		app.console.Open().Error << "Fail: " << exception.what() << "\n";
		exit_code = 1;
	}
	if (exit_code != 0) {
		app.console.Open().Output << "Shutting down due to failiure!";
	}
	return exit_code;
}

static void configure_application(ApplicationServices& app, int argc, char** argv, char** env)
{
	auto& di = app.dependency;

	di.For<IConsoleDriver>().UseType<Win32DefaultConsoleDriver>();
	app.console.UseDriver(app.dependency.GetInstance<IConsoleDriver>());
	di.For<Console>().UseSharedInstance(&app.console);
	di.For<AutoBuild>().UseDefaultConstructor();

	di.For<Configuration>().UseFactory([&] {
		return ConfigurationBuilder()
			.UseConfigurationFile("config.ini")
			.UseConsoleArgs(argc, argv)
			.UseEnvironment(env)
			.Build();
		});

	auto config = di.GetInstance<Configuration>();

	if (config->offline) {
		di.For<IPlaybackDevice>().UseType<OfflinePlaybackDevice>();
	}
	else if (config->musicEnabled) {
		di.For<IPlaybackDevice>().UseType<MusicPlaybackDevice>();
	}
	else {
		di.For<IPlaybackDevice>().UseType<ClockPlaybackDevice>();
	}

}
