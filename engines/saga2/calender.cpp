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

#include "saga2/saga2.h"
#include "saga2/calender.h"
#include "saga2/intrface.h"
#include "saga2/localize.h"
#include "saga2/saveload.h"


namespace Saga2 {

/* ===================================================================== *
   Globals
 * ===================================================================== */

static bool calenderPaused;

/* ===================================================================== *
   Constants
 * ===================================================================== */

const uint16 GAME_START_HOUR = 5;

/* ===================================================================== *
   FrameAlarm member functions
 * ===================================================================== */

void CalenderTime::read(Common::InSaveFile *in) {
	years = in->readUint16LE();
	weeks = in->readUint16LE();
	days = in->readUint16LE();
	dayInYear = in->readUint16LE();
	dayInWeek = in->readUint16LE();
	hour = in->readUint16LE();
	frameInHour = in->readUint16LE();

	debugC(3, kDebugSaveload, "... years = %d", years);
	debugC(3, kDebugSaveload, "... weeks = %d", weeks);
	debugC(3, kDebugSaveload, "... days = %d", days);
	debugC(3, kDebugSaveload, "... dayInYear = %d", dayInYear);
	debugC(3, kDebugSaveload, "... dayInWeek = %d", dayInWeek);
	debugC(3, kDebugSaveload, "... hour = %d", hour);
	debugC(3, kDebugSaveload, "... frameInHour = %d", frameInHour);
}

void CalenderTime::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint16LE(years);
	out->writeUint16LE(weeks);
	out->writeUint16LE(days);
	out->writeUint16LE(dayInYear);
	out->writeUint16LE(dayInWeek);
	out->writeUint16LE(hour);
	out->writeUint16LE(frameInHour);

	debugC(3, kDebugSaveload, "... years = %d", years);
	debugC(3, kDebugSaveload, "... weeks = %d", weeks);
	debugC(3, kDebugSaveload, "... days = %d", days);
	debugC(3, kDebugSaveload, "... dayInYear = %d", dayInYear);
	debugC(3, kDebugSaveload, "... dayInWeek = %d", dayInWeek);
	debugC(3, kDebugSaveload, "... hour = %d", hour);
	debugC(3, kDebugSaveload, "... frameInHour = %d", frameInHour);
}

void CalenderTime::update(void) {
	const char *text = NULL;

	if (++frameInHour >= kFramesPerHour) {
		frameInHour = 0;

		if (++hour >= kHoursPerDay) {
			hour = 0;

			days++;

			if (++dayInWeek >= kDaysPerWeek) {
				dayInWeek = 0;
				weeks++;
			}

			if (++dayInYear >= kDaysPerYear) {
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
	//  grows to 'kFramesAtNoon' at noon, then shrinks
	//  back to 0 at midnight again.
	solarAngle =    kFramesAtNoon
	                -   ABS(frameInDay() - kFramesAtNoon);

	//  Just for fun, we'll make the days longer in the summer,
	//  and shorter the winter. The calculation produces a number
	//  which equals kDaysPerYear/4 in summer, and -daysperYear/4
	//  in winter.
	season = kDaysPerYear / 4 - ABS(dayInYear - kDaysPerYear / 2);

	//  Convert season to an extra hour of daylight in summer,
	//  and an extra hour of night in winter. (That's an extra
	//  hour in the morning AND in the evening.
	season = season * kFramesPerHour / (kDaysPerYear / 4);

	//  This produces a triangle wave that goes from -kFramesAtNoon/3
	//  to kFramesAtNoon*2/3. Then we clip off the part of the
	//  curve below zero, and above 1/3, giving 1/3 night,
	//  1/6 morning, 1/3 day, and 1/6 evening.
	solarLevel = clamp(g_vm->_showNight ? /* 0 */ (kDayBias * 5 / 4) : (kFramesAtNoon / 3),
	                   solarAngle * 2 + season - kFramesAtNoon / 3 + kDayBias * 2,
	                   kFramesAtNoon / 3);

	return (solarLevel * maxLevel) / (kFramesAtNoon / 3);
}

/* ===================================================================== *
   FrameAlarm member functions
 * ===================================================================== */

void FrameAlarm::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint16LE(baseFrame);
	out->writeUint16LE(duration);
}

void FrameAlarm::read(Common::InSaveFile *in) {
	baseFrame = in->readUint16LE();
	duration = in->readUint16LE();
}

void FrameAlarm::set(uint16 dur) {
	baseFrame = calender.frameInDay();
	duration = dur;
}

bool FrameAlarm::check(void) {
	uint16      frameInDay = calender.frameInDay();

	return  baseFrame + duration < CalenderTime::kFramesPerDay
	        ?   frameInDay >= baseFrame + duration
	        :       frameInDay < baseFrame
	        &&  frameInDay >=       baseFrame
	        +   duration
	        -   CalenderTime::kFramesPerDay;
}

// time elapsed since alarm set

uint16 FrameAlarm::elapsed(void) {
	uint16      frameInDay = calender.frameInDay();

	return  baseFrame + duration < CalenderTime::kFramesPerDay
	        ?   frameInDay - baseFrame
	        :   frameInDay >= baseFrame
	        ?   frameInDay - baseFrame
	        :   frameInDay + CalenderTime::kFramesPerDay - baseFrame;
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

	time1Frame =    time1.days * CalenderTime::kFramesPerDay
	                +   time1.hour * CalenderTime::kFramesPerHour
	                +   time1.frameInHour;
	time2Frame =    time2.days * CalenderTime::kFramesPerDay
	                +   time2.hour * CalenderTime::kFramesPerHour
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

void saveCalender(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving calender");

	outS->write("CALE", 4);
	CHUNK_BEGIN;
	out->writeUint16LE(calenderPaused);
	debugC(3, kDebugSaveload, "... calenderPaused = %d", calenderPaused);
	calender.write(out);
	CHUNK_END;
}

void loadCalender(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading calender");

	calenderPaused = in->readUint16LE();

	debugC(3, kDebugSaveload, "... calenderPaused = %d", calenderPaused);

	calender.read(in);
}

CalenderTime    calender;

bool isDayTime(void) {
	return calender.lightLevel(MAX_LIGHT) >= (MAX_LIGHT / 2);
}

}
