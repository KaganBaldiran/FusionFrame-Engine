#include "StopWatch.h"


void FUSIONUTIL::Timer::Set()
{
	seconds = 0;
	mins = 0;
	hours = 0;
	clockchecker.set();
	start = std::chrono::steady_clock::now();
}

template<typename T>
T FUSIONUTIL::Timer::GetCurrentTime()
{
	return std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - start);
}

int FUSIONUTIL::Timer::GetSeconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
}

int FUSIONUTIL::Timer::GetMinutes()
{
	auto duration = std::chrono::steady_clock::now() - start;
	int totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	mins = (totalSeconds / 60) % 60;
	return mins;
}


int FUSIONUTIL::Timer::GetHours()
{
	auto duration = std::chrono::steady_clock::now() - start;
	int totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	hours = totalSeconds / 3600; 
	return hours;
}

float FUSIONUTIL::Timer::GetMicroseconds()
{
	auto duration = std::chrono::steady_clock::now() - start;
	microSeconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
	return microSeconds;
}

float FUSIONUTIL::Timer::GetMiliseconds()
{
	auto duration = std::chrono::steady_clock::now() - start;
	miliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
	return miliseconds + (GetMicroseconds() / 1000000.0f);
}


void FUSIONUTIL::Timer::Reset()
{
	Set();
}

