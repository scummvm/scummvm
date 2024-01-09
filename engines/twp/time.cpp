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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include <time.h>
#include "twp/time.h"

namespace Twp {

Common::String formatTime(int64_t t, const char *format) {
	time_t time = (time_t)t;
	struct tm *tm = localtime(&time);
	char buf[64];
	strftime(buf, 64, format, tm);
	return Common::String(buf);
}

DateTime toDateTime(int64_t t) {
	time_t time = (time_t)t;
	struct tm *tm = localtime(&time);
	DateTime dateTime;
	dateTime.year = 1900 + tm->tm_year;
	dateTime.month = 1 + tm->tm_mon;
	dateTime.day = tm->tm_mday;
	dateTime.hour = tm->tm_hour;
	dateTime.min = tm->tm_min;
	return dateTime;
}

} // namespace Twp
