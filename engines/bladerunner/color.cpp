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

#include "bladerunner/color.h"

namespace BladeRunner {

// This array essentially stores the conversion from unsigned 5bit values to 8bit
// ie. ((int)i * 255) / 31 (integer division), for i values 0 to 31	
// Note that just using a multiplier 256/16 (= 8) will not properly
// map the color, since eg. value 31 would be mapped to 248 instead of 255.
const uint8 Color::map5BitsTo8Bits[] = {0, 8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98, 106, 115, 123, 131, 139, 148, 156, 164, 172, 180, 189, 197, 205, 213, 222, 230, 238, 246, 255};

uint8 Color::get8BitColorFrom5Bit(uint8 col5b) {
	if (col5b > 31) {
		// A value larger than 31 is invalid (never going to happen for 5bits)
		// but still catch the case, since the parameter is 8bits
		return 255;
	}
	return map5BitsTo8Bits[col5b];
}
	
} // End of namespace BladeRunner