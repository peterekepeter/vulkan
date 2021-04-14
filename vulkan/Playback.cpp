#include "pch.h"
#include "Playback.hpp"
#include "Configuration.hpp"

MusicPlaybackDevice::MusicPlaybackDevice(DependencyManager* di)
{
	auto config = di->GetInstance<Configuration>();
	Music::Init(config->musicFile.c_str(), config->musicStream);
	this->position = 0.0;
}

void MusicPlaybackDevice::SetPosition(double seconds)
{
	auto bytes = seconds * 44100.0 * 2.0 * 2.0;
	Music::SetPositionByte(static_cast<long long>(bytes));
	this->position = seconds;
}

double MusicPlaybackDevice::GetPosition()
{
	return this->position;
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

void MusicPlaybackDevice::NextFrame()
{
	auto bytePosition = Music::GetPositionByte();
	this->position = bytePosition / 44100.0 / 2.0 / 2.0;
}

ClockPlaybackDevice::ClockPlaybackDevice()
{
	startOfMeasurementPosition = 0.0;
	position = 0.0;
	isPlaying = false;
	startOfMeasurement = std::chrono::steady_clock::now();
}

void ClockPlaybackDevice::SetPosition(double seconds)
{
	startOfMeasurement = std::chrono::steady_clock::now();
	startOfMeasurementPosition = seconds;
	position = seconds;
}

double ClockPlaybackDevice::GetPosition()
{
	return position;
}

bool ClockPlaybackDevice::IsPlaying()
{
	return isPlaying;
}

void ClockPlaybackDevice::Play()
{
	if (!isPlaying) {
		this->startOfMeasurementPosition = this->position;
		startOfMeasurement = std::chrono::steady_clock::now();
		isPlaying = true;
	}
}

void ClockPlaybackDevice::Pause()
{
	if (isPlaying) {
		isPlaying = false;
	}
}

void ClockPlaybackDevice::NextFrame()
{
	if (isPlaying) 
	{
		using namespace std::chrono;
		auto now = steady_clock::now();
		auto delta = duration_cast<microseconds>(now - startOfMeasurement);
		this->position = this->startOfMeasurementPosition + delta.count() * 1e-6;
	}
}

void OfflinePlaybackDevice::UpdatePosition()
{
	this->position = frame / fps;
}

OfflinePlaybackDevice::OfflinePlaybackDevice(DependencyManager* di)
{
	auto config = di->GetInstance<Configuration>();
	this->isPlaying = false;
	this->fps = config->fps;
	this->frame = 0;
	this->position = 0;
}

void OfflinePlaybackDevice::SetPosition(double seconds)
{
	this->frame = static_cast<int>(seconds * fps);
}

double OfflinePlaybackDevice::GetPosition()
{
	return position;
}

bool OfflinePlaybackDevice::IsPlaying()
{
	return this->isPlaying;
}

void OfflinePlaybackDevice::Play()
{
	this->isPlaying = true;
}

void OfflinePlaybackDevice::Pause()
{
	this->isPlaying = false;
}

void OfflinePlaybackDevice::NextFrame()
{
	if (this->isPlaying) 
	{
		UpdatePosition();
		this->frame++;
	}
}
