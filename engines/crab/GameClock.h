#pragma once

#include "common_header.h"
#include "timer.h"

class GameClock
{
	Timer timer;
	Uint32 start;
	std::string seperator;

public:
	GameClock() : seperator(" : "){ start = 0; }

	void Start(Uint32 initial_time = 0)
	{
		start = initial_time;
		timer.Start();
	}

	void Start(const std::string &str)
	{
		Uint32 ms = 0, hr = 0, min = 0, sec = 0;
		std::string str_hrs, str_min, str_sec;

		std::size_t found_1 = str.find_first_of(seperator);
		if(found_1 > 0 && found_1 != std::string::npos)
		{
			str_hrs = str.substr(0,found_1);
			hr = StringToNumber<Uint32>(str_hrs);

			std::size_t found_2 = str.find_last_of(seperator);
			if(found_2 > 0 && found_2 != std::string::npos)
			{
				str_sec = str.substr(found_2+1,std::string::npos);
				sec = StringToNumber<Uint32>(str_sec);

				str_min = str.substr(found_1+seperator.length(), found_2-(2*seperator.length()));
				min = StringToNumber<Uint32>(str_min);
			}
		}

		ms = 3600000 * hr + 60000 * min + 1000 * sec;
		Start(ms);
	}

	std::string GetTime()
	{
		Uint32 ms = start + timer.Ticks();

		Uint32 x = ms / 1000;
		Uint32 seconds = x % 60;
		x /= 60;
		Uint32 minutes = x % 60;
		Uint32 hours = x / 60;

		return NumberToString(hours) + seperator + NumberToString(minutes) + seperator + NumberToString(seconds);
	}
};