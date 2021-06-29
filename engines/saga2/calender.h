/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_CALENDER_H
#define SAGA2_CALENDER_H

namespace Saga2 {

class SaveFileConstructor;
class SaveFileReader;

/* ===================================================================== *
   CalenderTime class
 * ===================================================================== */

class CalenderTime {
public:
	enum {
		//  Basic constants
		daysPerWeek     = 7,
		daysPerYear     = 365,
		hoursPerDay     = 24,
		realMinutesPerDay = 30,

		//  Number of animation frames per day
		framesPerDay    = 10 * 60 * realMinutesPerDay,

		//  Derived variables
		framesPerHour   = (framesPerDay / hoursPerDay),
		framesAtNoon    = (framesPerDay / 2)
	};

	uint16      years,
	            weeks,
	            days,
	            dayInYear,
	            dayInWeek,
	            hour,
	            frameInHour;

	void update(void);
	int lightLevel(int maxLevel);

	uint16 frameInDay(void) {
		return hour * framesPerHour + frameInHour;
	}
};

/* ===================================================================== *
   FrameAlarm class
 * ===================================================================== */

class FrameAlarm {
	uint16      baseFrame,
	            duration;
public:
	void set(uint16 dur);
	bool check(void);
	uint16 elapsed(void);
};

/* ===================================================================== *
   Calender management functions
 * ===================================================================== */

void updateCalender(void);
void pauseCalender(void);
void resumeCalender(void);

uint32 operator - (const CalenderTime &time1, const CalenderTime &time2);

void initCalender(void);

void saveCalender(SaveFileConstructor &saveGame);
void loadCalender(SaveFileReader &saveGame);

bool isDayTime(void);

/* ===================================================================== *
   Global calender
 * ===================================================================== */

extern CalenderTime calender;

const int MAX_LIGHT = 12;       // maximum light level

}

#endif
