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

#include "ags/lib/system/datetime.h"

namespace AGS3 {

/*
 * Converts a date/time structure to Unix timestamp
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.0.0
 */
time_t convertDateToUnixTime(const TimeDate *date) {
	uint y;
	uint m;
	uint d;
	time_t t;

	// Year
	y = date->tm_year;
	// Month of year
	m = date->tm_mon;
	// Day of month
	d = date->tm_mday;

	// January and February are counted as months 13 and 14 of the previous year
	if (m <= 2) {
		m += 12;
		y -= 1;
	}

	// Convert years to days
	t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
	// Convert months to days
	t += (30 * m) + (3 * (m + 1) / 5) + d;
	// Unix time starts on January 1st, 1970
	t -= 719561;
	// Convert days to seconds
	t *= 86400;
	// Add hours, minutes and seconds
	t += (3600 * date->tm_hour) + (60 * date->tm_min) + date->tm_sec;

	// Return Unix time
	return t;
}

void localTime(tm *time) {
	g_system->getTimeAndDate(*time);

	time->tm_yday = 0;
	time->tm_isdst = 0;
}

time_t getUnixTime() {
	tm time;
	localTime(&time);

	return convertDateToUnixTime(&time);
}

} // namespace AGS3
