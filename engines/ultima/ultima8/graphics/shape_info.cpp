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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/shape_info.h"
#include "ultima/ultima8/kernel/core_app.h"

namespace Ultima8 {

bool ShapeInfo::getTypeFlag(int typeflag) {
	if (GAME_IS_U8)
		return getTypeFlagU8(typeflag);
	else if (GAME_IS_CRUSADER)
		return getTypeFlagCrusader(typeflag);
	CANT_HAPPEN_MSG("Invalid game type");
	return false;
}

bool ShapeInfo::getTypeFlagU8(int typeflag) {
	// This is not nice. The Typeflags in U8 were stored in an 8 byte array
	// and they could access them with a number from 0 to 63
	// Problem: We don't store them in an 8 byte array so we can't access
	// with a number from 0 to 63

	// So what we do is split the flag up into the bits

	if (typeflag <= 11) {       // flags        Byte 0, 1:0-3   Bits  0-11
		return (flags >> typeflag) & 1;
	} else if (typeflag <= 15) { // family       Byte 1:4-7      Bits 11-15
		return (family >> (typeflag - 12)) & 1;
	} else if (typeflag <= 19) { // equiptype    Byte 2:0-3      Bits 16-19
		return (equiptype >> (typeflag - 16)) & 1;
	} else if (typeflag <= 23) { // x            Byte 2:4-7      Bits 20-23
		return (x >> (typeflag - 20)) & 1;
	} else if (typeflag <= 27) { // y            Byte 3:0-3      Bits 24-27
		return (y >> (typeflag - 24)) & 1;
	} else if (typeflag <= 31) { // z            Byte 3:4-7      Bits 28-31
		return (z >> (typeflag - 28)) & 1;
	} else if (typeflag <= 35) { // animtype     Byte 4:0-3      Bits 32-35
		return (animtype >> (typeflag - 32)) & 1;
	} else if (typeflag <= 39) { // animdata     Byte 4:4-7      Bits 36-49
		return (animdata >> (typeflag - 36)) & 1;
	} else if (typeflag <= 43) { // unknown      Byte 5:0-3      Bits 40-43
		return (unknown >> (typeflag - 40)) & 1;
	} else if (typeflag <= 47) { // flags        Byte 5:4-7      Bits 44-47
		return (flags >> (12 + typeflag - 44)) & 1;
	} else if (typeflag <= 55) { // weight       Byte 6          Bits 48-55
		return (weight >> (typeflag - 48)) & 1;
	} else if (typeflag <= 63) { // volume       Byte 7          Bits 56-63
		return (volume >> (typeflag - 56)) & 1;
	}

	return false;
}

bool ShapeInfo::getTypeFlagCrusader(int typeflag) {
	if (typeflag <= 11) {       // flags        Byte 0, 1:0-3   Bits  0-11
		return (flags >> typeflag) & 1;
	} else if (typeflag <= 16) { // family       Byte 1:4-7,2:0  Bits 12-16
		return (family >> (typeflag - 12)) & 1;
	} else if (typeflag <= 20) { // unknown      Byte 2:0-3      Bits 17-20
		perr << "Warning: unknown typeflag requested." << std::endl;
	} else if (typeflag <= 26) { // x            Byte 2:4-7,3:0-1    Bits 21-26
		return (x >> (typeflag - 21)) & 1;
	} else if (typeflag <= 31) { // y            Byte 3:2-6      Bits 27-31
		return (y >> (typeflag - 27)) & 1;
	} else if (typeflag <= 36) { // z            Byte 3:7,4:0-3  Bits 32-36
		return (z >> (typeflag - 32)) & 1;
	} else if (typeflag <= 47) {
		perr << "Warning: unknown typeflag requested." << std::endl;
	} else if (typeflag <= 55) { // flags        Byte 6: 0-7     Bits 48-55
		return (flags >> (12 + typeflag - 55)) & 1;
	} else if (typeflag <= 71) {
		perr << "Warning: unknown typeflag requested." << std::endl;
	}

	return false;
}

} // End of namespace Ultima8
