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
 */

#ifndef GLK_TIME_H
#define GLK_TIME_H

#include "glk/glk_types.h"

namespace Glk {

typedef int64 TimeSeconds;

struct Timestamp {
	glsi32 high_sec;
	glui32 low_sec;
	glsi32 microsec;
};
typedef Timestamp glktimeval_t;

struct TimeAndDate {
	glsi32 year;     ///< full (four-digit) year
	glsi32 month;    ///< 1-12, 1 is January
	glsi32 day;      ///< 1-31
	glsi32 weekday;  ///< 0-6, 0 is Sunday
	glsi32 hour;     ///< 0-23
	glsi32 minute;   ///< 0-59
	glsi32 second;   ///< 0-59, maybe 60 during a leap second
	glsi32 microsec; ///< 0-999999
private:
	/**
	 * Get the number of seconds since the start of 1970
	 */
	int64 getSecondsSince1970() const;

	/**
	 * Convert a time in seconds to the structure
	 */
	void setTime(const TimeSeconds &ts);

	/**
	 * Get time in seconds from the structure
	 */
	TimeSeconds getTime() const;
public:
	/**
	 * Constructor
	 */
	TimeAndDate();

	/**
	 * Constructor
	 */
	TimeAndDate(const TimeSeconds &ts);

	/**
	 * Constructor
	 */
	TimeAndDate(const Timestamp &t);

	/**
	 * Convert to seconds
	 */
	operator TimeSeconds() const;

	/**
	 * Convert to time stamp
	 */
	operator Timestamp() const;
};
typedef TimeAndDate glkdate_t;

} // End of namespace Glk

#endif
