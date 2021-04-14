#include "pch.h"
#include "Stopwatch.h"

Stopwatch::Stopwatch()
{
	start_time = std::chrono::steady_clock::now();
}

int64_t Stopwatch::elapsed_milliseconds() {
	auto elapsed = std::chrono::steady_clock::now() - start_time;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	return elapsed_ms;
}