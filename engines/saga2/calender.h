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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_CALENDER_H
#define SAGA2_CALENDER_H

namespace Saga2 {

/* ===================================================================== *
   CalenderTime class
 * ===================================================================== */

class CalenderTime {
public:
	enum {
		//  Basic constants
		kDaysPerWeek     = 7,
		kDaysPerYear     = 365,
		kHoursPerDay     = 24,
		kRealMinutesPerDay = 30,

		//  Number of animation frames per day
		kFramesPerDay    = 10 * 60 * kRealMinutesPerDay,

		//  Derived variables
		kFramesPerHour   = (kFramesPerDay / kHoursPerDay),
		kFramesAtNoon    = (kFramesPerDay / 2),

		kDayBias = kFramesAtNoon / 6,

		kGameStartHour = 5
	};

	uint16      _years,
	            _weeks,
	            _days,
	            _dayInYear,
	            _dayInWeek,
	            _hour,
	            _frameInHour;

	bool _calenderPaused;

	CalenderTime() {
		_years = _weeks = _days = _dayInYear = _dayInWeek = _hour = _frameInHour = 0;
		_calenderPaused = false;
	}

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);

	void update();
	int lightLevel(int maxLevel);

	uint16 frameInDay() {
		return _hour * kFramesPerHour + _frameInHour;
	}
};

/* ===================================================================== *
   FrameAlarm class
 * ===================================================================== */

class FrameAlarm {
	uint16      _baseFrame,
	            _duration;
public:
	void set(uint16 dur);
	bool check();
	uint16 elapsed();

	void write(Common::MemoryWriteStreamDynamic *out);
	void read(Common::InSaveFile *in);
};

/* ===================================================================== *
   Calender management functions
 * ===================================================================== */

void updateCalendar();
void pauseCalender();
void resumeCalender();

uint32 operator - (const CalenderTime &time1, const CalenderTime &time2);

void initCalender();

void saveCalender(Common::OutSaveFile *outS);
void loadCalender(Common::InSaveFile *in);

bool isDayTime();

const int MAX_LIGHT = 12;       // maximum light level

}

#endif
