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

#include "saga2/saga2.h"
#include "saga2/calendar.h"
#include "saga2/intrface.h"
#include "saga2/localize.h"
#include "saga2/saveload.h"


namespace Saga2 {

/* ===================================================================== *
   FrameAlarm member functions
 * ===================================================================== */

void CalendarTime::read(Common::InSaveFile *in) {
	_years = in->readUint16LE();
	_weeks = in->readUint16LE();
	_days = in->readUint16LE();
	_dayInYear = in->readUint16LE();
	_dayInWeek = in->readUint16LE();
	_hour = in->readUint16LE();
	_frameInHour = in->readUint16LE();

	debugC(3, kDebugSaveload, "... _years = %d", _years);
	debugC(3, kDebugSaveload, "... _weeks = %d", _weeks);
	debugC(3, kDebugSaveload, "... _days = %d", _days);
	debugC(3, kDebugSaveload, "... _dayInYear = %d", _dayInYear);
	debugC(3, kDebugSaveload, "... _dayInWeek = %d", _dayInWeek);
	debugC(3, kDebugSaveload, "... _hour = %d", _hour);
	debugC(3, kDebugSaveload, "... _frameInHour = %d", _frameInHour);
}

void CalendarTime::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint16LE(_years);
	out->writeUint16LE(_weeks);
	out->writeUint16LE(_days);
	out->writeUint16LE(_dayInYear);
	out->writeUint16LE(_dayInWeek);
	out->writeUint16LE(_hour);
	out->writeUint16LE(_frameInHour);

	debugC(3, kDebugSaveload, "... _years = %d", _years);
	debugC(3, kDebugSaveload, "... _weeks = %d", _weeks);
	debugC(3, kDebugSaveload, "... _days = %d", _days);
	debugC(3, kDebugSaveload, "... _dayInYear = %d", _dayInYear);
	debugC(3, kDebugSaveload, "... _dayInWeek = %d", _dayInWeek);
	debugC(3, kDebugSaveload, "... _hour = %d", _hour);
	debugC(3, kDebugSaveload, "... _frameInHour = %d", _frameInHour);
}

void CalendarTime::update() {
	const char *text = nullptr;

	if (++_frameInHour >= kFramesPerHour) {
		_frameInHour = 0;

		if (++_hour >= kHoursPerDay) {
			_hour = 0;

			_days++;

			if (++_dayInWeek >= kDaysPerWeek) {
				_dayInWeek = 0;
				_weeks++;
			}

			if (++_dayInYear >= kDaysPerYear) {
				_dayInYear = 0;
				_years++;
			}
		}

		switch (_hour) {
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
			StatusMsg(CALENDAR_STATUS, text, _dayInWeek + 1, _weeks + 1);
	}
}

int CalendarTime::lightLevel(int maxLevel) {
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
	season = kDaysPerYear / 4 - ABS(_dayInYear - kDaysPerYear / 2);

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
	out->writeUint16LE(_baseFrame);
	out->writeUint16LE(_duration);
}

void FrameAlarm::read(Common::InSaveFile *in) {
	_baseFrame = in->readUint16LE();
	_duration = in->readUint16LE();
}

void FrameAlarm::set(uint16 dur) {
	_baseFrame = g_vm->_calendar->frameInDay();
	_duration = dur;
}

bool FrameAlarm::check() {
	uint16      frameInDay = g_vm->_calendar->frameInDay();

	return  _baseFrame + _duration < CalendarTime::kFramesPerDay
	        ?   frameInDay >= _baseFrame + _duration
	        :       frameInDay < _baseFrame
	        &&  frameInDay >=       _baseFrame
	        +   _duration
	        -   CalendarTime::kFramesPerDay;
}

// time elapsed since alarm set

uint16 FrameAlarm::elapsed() {
	uint16      frameInDay = g_vm->_calendar->frameInDay();

	return  _baseFrame + _duration < CalendarTime::kFramesPerDay
	        ?   frameInDay - _baseFrame
	        :   frameInDay >= _baseFrame
	        ?   frameInDay - _baseFrame
	        :   frameInDay + CalendarTime::kFramesPerDay - _baseFrame;
}

/* ===================================================================== *
   Calendar management functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Pause the global calendar

void pauseCalendar() {
	g_vm->_calendar->_calendarPaused = true;
}

//-----------------------------------------------------------------------
//	Restart the paused global calendar

void resumeCalendar() {
	g_vm->_calendar->_calendarPaused = false;
}

//-----------------------------------------------------------------------
//	Update the global calendar

void updateCalendar() {
	if (!g_vm->_calendar->_calendarPaused) g_vm->_calendar->update();
}

//-----------------------------------------------------------------------
//	CalendarTime subtraction operator

uint32 operator - (const CalendarTime &time1, const CalendarTime &time2) {
	uint32      time1Frame,
	            time2Frame;

	time1Frame =    time1._days * CalendarTime::kFramesPerDay
	                +   time1._hour * CalendarTime::kFramesPerHour
	                +   time1._frameInHour;
	time2Frame =    time2._days * CalendarTime::kFramesPerDay
	                +   time2._hour * CalendarTime::kFramesPerHour
	                +   time2._frameInHour;

	return time1Frame - time2Frame;
}

//-----------------------------------------------------------------------
//	Initialize the game calendar

void initCalendar() {
	g_vm->_calendar->_calendarPaused          = false;
	g_vm->_calendar->_years          = 0;
	g_vm->_calendar->_weeks          = 0;
	g_vm->_calendar->_days           = 0;
	g_vm->_calendar->_dayInYear      = 0;
	g_vm->_calendar->_dayInWeek      = 0;
	g_vm->_calendar->_hour           = CalendarTime::kGameStartHour;
	g_vm->_calendar->_frameInHour    = 0;
}

void saveCalendar(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving calendar");

	outS->write("CALE", 4);
	CHUNK_BEGIN;
	out->writeUint16LE(g_vm->_calendar->_calendarPaused);
	debugC(3, kDebugSaveload, "... _calendarPaused = %d", g_vm->_calendar->_calendarPaused);
	g_vm->_calendar->write(out);
	CHUNK_END;
}

void loadCalendar(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading calendar");

	g_vm->_calendar->_calendarPaused = in->readUint16LE();

	debugC(3, kDebugSaveload, "... _calendarPaused = %d", g_vm->_calendar->_calendarPaused);

	g_vm->_calendar->read(in);
}

bool isDayTime() {
	return g_vm->_calendar->lightLevel(MAX_LIGHT) >= (MAX_LIGHT / 2);
}

}
