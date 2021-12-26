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

#include "glk/archetype/timestamp.h"
#include "common/system.h"

namespace Glk {
namespace Archetype {

TimestampType GTimeStamp;

void timestamp_init() {
	GTimeStamp = 0;
}

void get_time_stamp(TimestampType &tstamp) {
	// Get the time and date
	TimeDate td;
	g_system->getTimeAndDate(td);

	// Normalize the year
	tstamp = ((td.tm_year - 1992) % 64) << 26;
	tstamp |= td.tm_mon << 22;
	tstamp |= td.tm_mday << 17;
	tstamp |= td.tm_hour << 12;
	tstamp |= td.tm_min << 6;
	tstamp |= td.tm_sec;
}

} // End of namespace Archetype
} // End of namespace Glk
