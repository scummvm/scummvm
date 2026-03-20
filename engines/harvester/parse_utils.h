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

#ifndef HARVESTER_PARSE_UTILS_H
#define HARVESTER_PARSE_UTILS_H

#include <climits>

#include "common/str.h"
#include "common/util.h"

namespace Harvester {

// Mirrors atoi's permissive decimal parsing while relying on Common helpers.
inline int parseAsciiIntOrZero(const Common::String &value) {
	uint start = 0;
	while (start < value.size() && Common::isSpace((byte)value[start]))
		++start;

	bool negative = false;
	if (start < value.size() && (value[start] == '+' || value[start] == '-')) {
		negative = value[start] == '-';
		++start;
	}

	uint end = start;
	while (end < value.size() && Common::isDigit((byte)value[end]))
		++end;

	if (start == end)
		return 0;

	const uint64 magnitude = value.substr(start, end - start).asUint64();
	if (negative) {
		if (magnitude > (uint64)INT_MAX + 1)
			return INT_MIN;
		return -(int)magnitude;
	}

	if (magnitude > (uint64)INT_MAX)
		return INT_MAX;
	return (int)magnitude;
}

} // End of namespace Harvester

#endif
