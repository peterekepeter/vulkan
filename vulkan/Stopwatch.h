#pragma once

class Stopwatch
{
	std::chrono::steady_clock::time_point start_time;

public:
	Stopwatch();
	int64_t elapsed_milliseconds();
};
