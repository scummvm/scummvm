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

#ifndef TWP_TIME_H
#define TWP_TIME_H

#include "common/str.h"

namespace Twp {

/// @brief Convert a time value (number of seconds since the Unix epoch) to a human-readable string format.
/// @param t The time value to format, as a number of seconds since the Unix epoch (1970-01-01 00:00:00 UTC).
/// @return A formatted string representing the date and time, in the format "Mon DD at HH:MM", where Mon is the three-letter month abbreviation, DD is the day of the month, HH is the hour (00-23), and MM is the minute (00-59).
Common::String formatTime(int64 t);

/// @brief Convert a time value (number of seconds since the Unix epoch) to a TimeDate struct representing the corresponding date and time components.
/// @param t The time value to convert, as a number of seconds since the Unix epoch (1970-01-01 00:00:00 UTC).
/// @return A TimeDate struct representing the date and time components.
TimeDate intToTimeDate(int64 t);

/// @brief Get the current time as a time value (number of seconds since the Unix epoch).
/// @return The current time as a number of seconds since the Unix epoch (1970-01-01 00:00:00 UTC).
int64 getTime();

} // namespace Twp

#endif
