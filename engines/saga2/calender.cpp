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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/calender.h"
#include "saga2/intrface.h"
#include "saga2/localize.h"
#include "saga2/savefile.h"

namespace Saga2 {

/* ===================================================================== *
   Globals
 * ===================================================================== */

static bool calenderPaused;

const int dayBias = CalenderTime::framesAtNoon / 6;

/* ===================================================================== *
   Constants
 * ===================================================================== */

const uint16 GAME_START_HOUR = 5;

/* ===================================================================== *
   FrameAlarm member functions
 * ===================================================================== */

void CalenderTime::update(void) {
	const char *text = NULL;

	if (++frameInHour >= framesPerHour) {
		frameInHour = 0;

		if (++hour >= hoursPerDay) {
			hour = 0;

			days++;

			if (++dayInWeek >= daysPerWeek) {
				dayInWeek = 0;
				weeks++;
			}

			if (++dayInYear >= daysPerYear) {
				dayInYear = 0;
				years++;
			}
		}

		switch (hour) {
		case 0:
			text = HOUR00_TIME;
			break;
		case 2:
			text = HOUR02_TIME;
			break;
		case 6:
			text = HOUR06_TIME;
			break;
		case 12:
			text = HOUR12_TIME;
			break;
		case 18:
			text = HOUR18_TIME;
			break;
		case 22:
			text = HOUR22_TIME;
			break;
		}

		if (text)
			StatusMsg(CALENDAR_STATUS, text, dayInWeek + 1, weeks + 1);
	}
}

int CalenderTime::lightLevel(int maxLevel) {
	int16       solarAngle,
	            season,
	            solarLevel;

	//  solarAngle equals starts at 0 at midnight, then linearly
	//  grows to 'framesAtNoon' at noon, then shrinks
	//  back to 0 at midnight again.
	solarAngle =    framesAtNoon
	                -   abs(frameInDay() - framesAtNoon);

	//  Just for fun, we'll make the days longer in the summer,
	//  and shorter the winter. The calculation produces a number
	//  which equals daysPerYear/4 in summer, and -daysperYear/4
	//  in winter.
	season = daysPerYear / 4 - abs(dayInYear - daysPerYear / 2);

	//  Convert season to an extra hour of daylight in summer,
	//  and an extra hour of night in winter. (That's an extra
	//  hour in the morning AND in the evening.
	season = season * framesPerHour / (daysPerYear / 4);

	//  This produces a triangle wave that goes from -framesAtNoon/3
	//  to framesAtNoon*2/3. Then we clip off the part of the
	//  curve below zero, and above 1/3, giving 1/3 night,
	//  1/6 morning, 1/3 day, and 1/6 evening.
	solarLevel = clamp(g_vm->_showNight ? /* 0 */ (dayBias * 5 / 4) : (framesAtNoon / 3),
	                   solarAngle * 2 + season - framesAtNoon / 3 + dayBias * 2,
	                   framesAtNoon / 3);

	return (solarLevel * maxLevel) / (framesAtNoon / 3);
}

/* ===================================================================== *
   FrameAlarm member functions
 * ===================================================================== */

void FrameAlarm::set(uint16 dur) {
	baseFrame = calender.frameInDay();
	duration = dur;
}

bool FrameAlarm::check(void) {
	uint16      frameInDay = calender.frameInDay();

	return  baseFrame + duration < CalenderTime::framesPerDay
	        ?   frameInDay >= baseFrame + duration
	        :       frameInDay < baseFrame
	        &&  frameInDay >=       baseFrame
	        +   duration
	        -   CalenderTime::framesPerDay;
}

// time elapsed since alarm set

uint16 FrameAlarm::elapsed(void) {
	uint16      frameInDay = calender.frameInDay();

	return  baseFrame + duration < CalenderTime::framesPerDay
	        ?   frameInDay - baseFrame
	        :   frameInDay >= baseFrame
	        ?   frameInDay - baseFrame
	        :   frameInDay + CalenderTime::framesPerDay - baseFrame;
}

/* ===================================================================== *
   Calender management functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Pause the global calender

void pauseCalender(void) {
	calenderPaused = true;
}

//-----------------------------------------------------------------------
//	Restart the paused global calender

void resumeCalender(void) {
	calenderPaused = false;
}

//-----------------------------------------------------------------------
//	Update the global calender

void updateCalender(void) {
	if (!calenderPaused) calender.update();
}

//-----------------------------------------------------------------------
//	CalenderTime subtraction operator

uint32 operator - (const CalenderTime &time1, const CalenderTime &time2) {
	uint32      time1Frame,
	            time2Frame;

	time1Frame =    time1.days * CalenderTime::framesPerDay
	                +   time1.hour * CalenderTime::framesPerHour
	                +   time1.frameInHour;
	time2Frame =    time2.days * CalenderTime::framesPerDay
	                +   time2.hour * CalenderTime::framesPerHour
	                +   time2.frameInHour;

	return time1Frame - time2Frame;
}

//-----------------------------------------------------------------------
//	Initialize the game calender

void initCalender(void) {
	calenderPaused          = false;
	calender.years          = 0;
	calender.weeks          = 0;
	calender.days           = 0;
	calender.dayInYear      = 0;
	calender.dayInWeek      = 0;
	calender.hour           = GAME_START_HOUR;
	calender.frameInHour    = 0;
}

//-----------------------------------------------------------------------
//	Write the calender data to a save file

void saveCalender(SaveFileConstructor &saveGame) {
	if (saveGame.newChunk(
	            MKTAG('C', 'A', 'L', 'E'),
	            sizeof(calenderPaused) + sizeof(calender))) {
		saveGame.write(&calenderPaused, sizeof(calenderPaused));
		saveGame.write(&calender, sizeof(calender));
	}
}

//-----------------------------------------------------------------------
//	Read the calender data from a save file.  Assume the save file is at
//	the correct chunk.

void loadCalender(SaveFileReader &saveGame) {
	assert(saveGame.getChunkSize() ==  sizeof(calenderPaused) + sizeof(calender));

	saveGame.read(&calenderPaused, sizeof(calenderPaused));
	saveGame.read(&calender, sizeof(calender));
}

CalenderTime    calender;

bool isDayTime(void) {
	return calender.lightLevel(MAX_LIGHT) >= (MAX_LIGHT / 2);
}

}
