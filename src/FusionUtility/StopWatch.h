#pragma once
#include <chrono>
#include <bitset>
#include "FusionDLLExport.h"

namespace FUSIONUTIL
{
	class FUSIONFRAME_EXPORT Timer
	{
	public:
		void Set();

		template<typename T>
		T GetCurrentTime();
		int GetSeconds();
		int GetMinutes();
		int GetHours();
		float GetMiliseconds();
		float GetMicroseconds();
		void Reset();

		template <typename T, typename Rep, typename Period>
		T duration_cast(const std::chrono::duration<Rep, Period>& duration)
		{
			return duration.count() * static_cast<T>(Period::num) / static_cast<T>(Period::den);
		}

	private:

		std::chrono::steady_clock::time_point start;
		int mins;
		int hours;
		int seconds;
		float miliseconds;
		float microSeconds;
		std::bitset<2> clockchecker;
	};
}
