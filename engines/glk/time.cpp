/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/time.h"
#include "common/system.h"

namespace Glk {

TimeAndDate::TimeAndDate() {
	::TimeDate t;
	g_system->getTimeAndDate(t);

	year = t.tm_year;
	month = t.tm_mon;
	day = t.tm_mday;
	weekday = t.tm_wday;
	hour = t.tm_hour;
	minute = t.tm_min;
	second = t.tm_sec;
	microsec = 0;
}

TimeAndDate::TimeAndDate(const TimeSeconds &ts) {
	setTime(ts);
}

TimeAndDate::TimeAndDate(const Timestamp &t) {
	setTime(((int64)t.high_sec << 32) | t.low_sec);
}

TimeAndDate::operator TimeSeconds() const {
	return getTime();
}

TimeAndDate::operator Timestamp() const {
	TimeSeconds secs = getTime();
	Timestamp ts;
	ts.high_sec = secs >> 32;
	ts.low_sec = secs & 0xffffffff;
	ts.microsec = 0;

	return ts;
}

void TimeAndDate::setTime(const TimeSeconds &ts) {
	TimeSeconds total = ts;
	int daysInYear = 0, secsInYear = 0;

	// Figure out the year
	this->year = 1969;
	do {
		++this->year;
		total -= secsInYear;

		daysInYear = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 366 : 365;
		secsInYear = daysInYear * 24 * 60 * 60;
	} while (total >= secsInYear);

	// Figure out month and day
	int dayInYear = total / (24 * 60 * 60);
	total %= 24 * 60 * 60;

	int MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	this->month = 1;
	while (dayInYear >= MONTH_DAYS[this->month - 1]) {
		dayInYear -= MONTH_DAYS[this->month - 1];
		this->month++;
	}

	this->day = dayInYear + 1;

	// set the time within the day
	this->hour = total / (60 * 60);
	total %= (60 * 60);
	this->minute = total / 60;
	this->second = total % 60;
	this->microsec = 0;
}

TimeSeconds TimeAndDate::getTime() const {
	uint32 days = day - 1;
	for (int i = 1970; i < year; ++i)
		if ((i % 4 == 0 && i % 100 != 0) || (i % 400 == 0))
			days += 366;
		else
			days += 365;

	int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	for (int i = 1; i < month; ++i) {
		days += mdays[i - 1];
		if (i == 2)
			if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
				days += 1;
	}

	int64 totalHours = days * 24 + hour;
	int64 totalMinutes = totalHours * 60 + minute;
	return totalMinutes * 60 + second;
}

} // End of namespace Glk
