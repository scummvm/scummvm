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

namespace Ultima {
namespace Ultima8 {

bool ShapeInfo::getTypeFlag(int typeFlag) const {
	if (GAME_IS_U8)
		return getTypeFlagU8(typeFlag);
	else if (GAME_IS_CRUSADER)
		return getTypeFlagCrusader(typeFlag);
	CANT_HAPPEN_MSG("Invalid game type");
	return false;
}

bool ShapeInfo::getTypeFlagU8(int typeFlag) const {
	// This is not nice. The Typeflags in U8 were stored in an 8 byte array
	// and they could access them with a number from 0 to 63
	// Problem: We don't store them in an 8 byte array so we can't access
	// with a number from 0 to 63

	// So what we do is split the flag up into the bits

	if (typeFlag <= 11) {       // flags        Byte 0, 1:0-3   Bits  0-11
		return (_flags >> typeFlag) & 1;
	} else if (typeFlag <= 15) { // family       Byte 1:4-7      Bits 11-15
		return (_family >> (typeFlag - 12)) & 1;
	} else if (typeFlag <= 19) { // equipType    Byte 2:0-3      Bits 16-19
		return (_equipType >> (typeFlag - 16)) & 1;
	} else if (typeFlag <= 23) { // x            Byte 2:4-7      Bits 20-23
		return (_x >> (typeFlag - 20)) & 1;
	} else if (typeFlag <= 27) { // y            Byte 3:0-3      Bits 24-27
		return (_y >> (typeFlag - 24)) & 1;
	} else if (typeFlag <= 31) { // z            Byte 3:4-7      Bits 28-31
		return (_z >> (typeFlag - 28)) & 1;
	} else if (typeFlag <= 35) { // animtype     Byte 4:0-3      Bits 32-35
		return (_animType >> (typeFlag - 32)) & 1;
	} else if (typeFlag <= 39) { // animdata     Byte 4:4-7      Bits 36-49
		return (_animData >> (typeFlag - 36)) & 1;
	} else if (typeFlag <= 43) { // unknown      Byte 5:0-3      Bits 40-43
		return (_animSpeed >> (typeFlag - 40)) & 1;
	} else if (typeFlag <= 47) { // _flags       Byte 5:4-7      Bits 44-47
		return (_flags >> (12 + typeFlag - 44)) & 1;
	} else if (typeFlag <= 55) { // weight       Byte 6          Bits 48-55
		return (_weight >> (typeFlag - 48)) & 1;
	} else if (typeFlag <= 63) { // volume       Byte 7          Bits 56-63
		return (_volume >> (typeFlag - 56)) & 1;
	}

	return false;
}

bool ShapeInfo::getTypeFlagCrusader(int typeFlag) const {
	if (typeFlag <= 11) {       // _flags        Byte 0, 1:0-3   Bits  0-11
		return (_flags >> typeFlag) & 1;
	} else if (typeFlag <= 16) { // _family       Byte 1:4-7,2:0  Bits 12-16
		return (_family >> (typeFlag - 12)) & 1;
	} else if (typeFlag <= 20) { // unknown      Byte 2:0-3      Bits 17-20
		perr << "Warning: unknown typeFlag " << typeFlag << " requested." << Std::endl;
	} else if (typeFlag <= 26) { // x            Byte 2:4-7,3:0-1    Bits 21-26
		return (_x >> (typeFlag - 21)) & 1;
	} else if (typeFlag <= 31) { // y            Byte 3:2-6      Bits 27-31
		return (_y >> (typeFlag - 27)) & 1;
	} else if (typeFlag <= 36) { // z            Byte 3:7,4:0-3  Bits 32-36
		return (_z >> (typeFlag - 32)) & 1;
	} else if (typeFlag <= 47) {
		perr << "Warning: unknown typeFlag " << typeFlag << " requested." << Std::endl;
	} else if (typeFlag <= 55) { // _flags        Byte 6: 0-7     Bits 48-55
		return (_flags >> (12 + typeFlag - 55)) & 1;
	} else if (typeFlag <= 71) {
		perr << "Warning: unknown typeFlag " << typeFlag << " requested." << Std::endl;
	}

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
