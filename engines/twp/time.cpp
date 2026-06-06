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

#include "common/system.h"
#include "twp/time.h"

namespace Twp {

enum {
	SECONDS_IN_DAY = 86400, // number of seconds in a day
	SECONDS_IN_HOUR = 3600,// number of seconds in an hour
	SECONDS_IN_MINUTE = 60
};

Common::String formatTime(int64 t) {
	const char* monthList[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	const TimeDate timeDate = intToTimeDate(t);
	return Common::String::format( "%s %d at %.2d:%.2d", monthList[timeDate.tm_mon], timeDate.tm_mday, timeDate.tm_hour, timeDate.tm_min);
}

static bool isLeapYear(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int64_t dateTimeToInt64(const TimeDate &timeDate) {
	const int64_t year  = (int64_t)timeDate.tm_year + 1900; // fixup year
	const int month = timeDate.tm_mon;
	const int days_in_month[2][12] = {
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, // non-leap year
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } // leap year, February has 29 days
	};

	// Count days from the Unix epoch (1970-01-01) to Jan 1 of `year`
	/*
	 * For a year Y, the number of leap years in [1, Y-1] is:
	 *   floor((Y-1)/4) - floor((Y-1)/100) + floor((Y-1)/400)
	 * Days from year 0 to Jan 1 of Y = 365*Y + leaps
	 * Subtract the same value for 1970 to get the offset.
	 */
	const int64_t y0     = year  - 1;
	const int64_t y1970  = 1969; // 1970 - 1

	const int64_t leaps0    = y0    / 4 - y0    / 100 + y0    / 400;
	const int64_t leaps1970 = y1970 / 4 - y1970 / 100 + y1970 / 400;

	int64_t days = 365 * (year - 1970) + (leaps0 - leaps1970);

	// add days for the months elapsed in the current year
	const int leap = isLeapYear((int)year);
	for (int m = 0; m < month; m++)
		days += days_in_month[leap][m];

	// add remaining date/time fields
	days += (int64_t)timeDate.tm_mday - 1;

	const int64_t seconds = days * SECONDS_IN_DAY
		+ timeDate.tm_hour * SECONDS_IN_HOUR
		+ timeDate.tm_min  * SECONDS_IN_MINUTE
		+ timeDate.tm_sec;

	return seconds;
}

TimeDate intToTimeDate(int64 t) {
	TimeDate dateTime;
	int64_t seconds = t;
	dateTime.tm_sec = seconds % SECONDS_IN_MINUTE;
	seconds /= SECONDS_IN_MINUTE;
	dateTime.tm_min = seconds % 60;
	seconds /= 60;
	dateTime.tm_hour = seconds % 24;
	seconds /= 24;

	// Now `seconds` is the number of days since the Unix epoch (1970-01-01)
	// We can find the year by subtracting the number of days in each year until we find the correct one.
	int year = 1970;
	while (true) {
		const int days_in_year = isLeapYear(year) ? 366 : 365;
		if (seconds >= days_in_year) {
			seconds -= days_in_year;
			year++;
			continue;
		}
		break;
	}
	dateTime.tm_year = year - 1900; // fixup year

	// Now `seconds` is the number of days elapsed in the current year. We can find the month and day similarly.
	const int days_in_month[2][12] = {
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, // non-leap year
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }  // leap-year, February has 29 days
	};
	const int leap = isLeapYear(year);
	int month = 0;
	while (month < 12) {
		const int dim = days_in_month[leap][month];
		if (seconds >= dim) {
			seconds -= dim;
			month++;
		} else {
			break;
		}
	}
	dateTime.tm_mon = month; // tm_mon is 0-based
	dateTime.tm_mday = (int)seconds + 1;

	return dateTime;
}

int64 getTime() {
	TimeDate dateTimeTm;
	g_system->getTimeAndDate(dateTimeTm);
	return dateTimeToInt64(dateTimeTm);
}

} // namespace Twp
