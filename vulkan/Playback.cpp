#include "stdafx.h"
#include "Playback.hpp"
#include "Configuration.hpp"

MusicPlaybackDevice::MusicPlaybackDevice(Configuration* config)
{
	Music::Init(config->musicFile.c_str(), config->musicStream);
}

void MusicPlaybackDevice::SetPosition(double seconds)
{
	auto bytes = seconds * 44100.0 * 2.0 * 2.0;
	Music::SetPositionByte(static_cast<long long>(bytes));
}

double MusicPlaybackDevice::GetPosition()
{
	auto position = Music::GetPositionByte();
	auto seconds = position / 44100.0 / 2.0 / 2.0;
	return seconds;
}

bool MusicPlaybackDevice::IsPlaying()
{
	return Music::IsPlaying();
}

void MusicPlaybackDevice::Play()
{
	Music::Play();
}

void MusicPlaybackDevice::Pause()
{
	Music::Stop();
}

ClockPlaybackDevice::ClockPlaybackDevice()
{
	position = 0.0;
	isPlaying = false;
	startOfMeasurement = std::chrono::steady_clock::now();
}

void ClockPlaybackDevice::SetPosition(double seconds)
{
	startOfMeasurement = std::chrono::steady_clock::now();
	position = seconds;
}

double ClockPlaybackDevice::GetPosition()
{
	using namespace std::chrono;
	auto now = steady_clock::now();
	auto delta = duration_cast<microseconds>(now - startOfMeasurement);
	return position + delta.count() * 1e-6;
}

bool ClockPlaybackDevice::IsPlaying()
{
	return isPlaying;
}

void ClockPlaybackDevice::Play()
{
	if (!isPlaying) {
		startOfMeasurement = std::chrono::steady_clock::now();
		isPlaying = true;
	}
}

void ClockPlaybackDevice::Pause()
{
	if (isPlaying) {
		position = GetPosition();
		isPlaying = false;
	}
}
