#pragma once
#include "../submodule/app-service-sandwich/AppServiceSandwich/ApplicationServices.hpp"

struct RunResult
{
	bool requested_reload = false;
};

RunResult run_application(ApplicationServices& app);
