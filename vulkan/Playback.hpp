#pragma once
#include "Configuration.hpp"

class IPlaybackDevice
{
public:
	virtual void SetPosition(double seconds) = 0;
	virtual double GetPosition() = 0;
	virtual bool IsPlaying() = 0;
	virtual void Play() = 0;
	virtual void Pause() = 0;
};

class MusicPlaybackDevice : public IPlaybackDevice
{
public:
	MusicPlaybackDevice(Configuration* config);
	virtual void SetPosition(double seconds) override;
	virtual double GetPosition() override;
	virtual bool IsPlaying() override;
	virtual void Play() override;
	virtual void Pause() override;
};

class ClockPlaybackDevice : public IPlaybackDevice 
{
	bool isPlaying;
	double position;
	std::chrono::steady_clock::time_point startOfMeasurement;
public:
	ClockPlaybackDevice();
	virtual void SetPosition(double seconds) override;
	virtual double GetPosition() override;
	virtual bool IsPlaying() override;
	virtual void Play() override;
	virtual void Pause() override;
};