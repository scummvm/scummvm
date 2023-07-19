/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_GAMECLOCK_H
#define CRAB_GAMECLOCK_H

#include "crab/common_header.h"
#include "crab/timer.h"

namespace Crab {

class GameClock {
	Timer _timer;
	uint32 _start;
	Common::String _seperator;

public:
	GameClock() : _seperator(" : ") { _start = 0; }

	void start(uint32 initialTime = 0) {
		_start = initialTime;
		_timer.start();
	}

	void start(const Common::String &str) {
		uint32 ms = 0, hr = 0, min = 0, sec = 0;
		Common::String strHrs, strMin, strSec;

		size_t found1 = str.findFirstOf(_seperator);
		if (found1 > 0 && found1 != Common::String::npos) {
			strHrs = str.substr(0, found1);
			hr = stringToNumber<uint32>(strHrs);

			size_t found2 = str.findFirstOf(_seperator);
			if (found2 > 0 && found2 != Common::String::npos) {
				strSec = str.substr(found2 + 1, Common::String::npos);
				sec = stringToNumber<uint32>(strSec);

				strMin = str.substr(found1 + _seperator.size(), found2 - (2 * _seperator.size()));
				min = stringToNumber<uint32>(strMin);
			}
		}

		ms = 3600000 * hr + 60000 * min + 1000 * sec;
		start(ms);
	}

	Common::String getTime() {
		uint32 ms = _start + _timer.ticks();

		uint32 x = ms / 1000;
		uint32 seconds = x % 60;
		x /= 60;
		uint32 minutes = x % 60;
		uint32 hours = x / 60;

		Common::String timeStr = numberToString(hours);
		timeStr += _seperator;
		timeStr += numberToString(minutes);
		timeStr += _seperator;
		timeStr += numberToString(seconds);

		return timeStr;
	}
};

} // End of namespace Crab

#endif // CRAB_GAMECLOCK_H
