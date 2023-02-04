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

#include "ultima/ultima8/misc/common_types.h"

#include "ultima/ultima8/world/fire_type_table.h"
#include "ultima/ultima8/world/fire_type.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

const FireType *_getFireTypeRemorse(uint16 type) {
	static const FireType FIRE_TYPE_TABLE_REM[] = {
		// Extracted from CRUSADER.EXE 1478:252a
		//       FType      Hign Dmg  Num Shots  Sh.Mask  Cells/Rnd  Near Sprite
		//            Low Dmg     Range   Shield Cost Accurat  Rnd Dur
		FireType(0x0, 0,    0,    0,  0,  0,     0,   0,  0,   0,    0),
		FireType(0x1, 0x12, 0x19, 0,  1,  0x82,  0x7, 0,  0xc, 1000, 0),
		FireType(0x2, 0xa,  0x11, 0,  8,  0x50,  0x7, 0,  0xc, 1000, 0),
		FireType(0x3, 0xc8, 0xc8, 10, 1,  0,     0,   1,  0x3, 1000, 1),
		FireType(0x4, 0x96, 0x96, 10, 1,  0,     0,   0,  0x1, 1000, 1),
		FireType(0x5, 0x19, 0x2d, 0,  1,  0x190, 0x6, 0,  0x4, 1000, 1),
		FireType(0x6, 0xf,  0x23, 0,  1,  0x177, 0x6, 1,  0x4, 1000, 1),
		FireType(0x7, 0x5,  0x5,  0,  1,  0,     0,   0,  0x1, 1000, 1),
		FireType(0x8, 0x5,  0x5,  0,  1,  0,     0,   0,  0x1, 1000, 0),
		FireType(0x9, 0xa,  0xf,  0,  1,  0x12c, 0x6, 1,  0x1, 100,  1),
		FireType(0xa, 0x2d, 0x50, 10, 3,  0,     0,   0,  0x3, 100,  1),
		FireType(0xb, 0xe,  0x14, 0,  1,  0x50,  0x7, 0,  0xc, 1000, 0),
		FireType(0xc, 0x5,  0x5,  0,  1,  0,     0,   0,  0x1, 1000, 0),
		FireType(0xd, 0xa,  0x11, 0,  5,  0x50,  0x7, 0,  0xc, 1000, 0),
		FireType(0xe, 0xfa, 0xfa, 4,  1,  0x9c4, 0x4, 1,  0x3, 1000, 1),
		FireType(0xf, 0x23, 0x37, 4,  1,  0x2ee, 0x4, 0,  0x3, 1000, 1),
	};
	if (type >= ARRAYSIZE(FIRE_TYPE_TABLE_REM))
		return nullptr;
	return &FIRE_TYPE_TABLE_REM[type];
}

const FireType *_getFireTypeRegret(uint16 type) {
	static const FireType FIRE_TYPE_TABLE_REG[] = {
		// Extracted from REGRET.EXE 1480:2b2e
		FireType(0x0,  0,    0,    0,  0,  0,     0,   0, 0,   0,    0),
		FireType(0x1,  0x12, 0x19, 0,  1,  0x82,  0x7, 0, 0xc, 1000, 0),
		FireType(0x2,  0xf,  0x14, 0,  10, 0x50,  0x7, 0, 0xc, 1000, 0),
		FireType(0x3,  0xc8, 0xc8, 10, 1,  0x0,   0x0, 1, 0x3, 1000, 1),
		FireType(0x4,  0x96, 0x96, 10, 1,  0x0,   0x0, 0, 0x1, 1000, 1),
		FireType(0x5,  0x2d, 0x3c, 0,  1,  0x190, 0x6, 0, 0x4, 1000, 1),
		FireType(0x6,  0x19, 0x1e, 0,  1,  0x12c, 0x6, 1, 0x4, 1000, 1),
		FireType(0x7,  0x5,  0x5,  0,  1,  0x0,   0x0, 0, 0x1, 1000, 1),
		FireType(0x8,  0x5,  0x5,  0,  1,  0x0,   0x0, 0, 0x1, 1000, 0),
		FireType(0x9,  0xa,  0xf,  0,  1,  0x12c, 0x6, 1, 0x1, 100,  1),
		FireType(0xa,  0x2d, 0x50, 7,  3,  0x0,   0x0, 0, 0x3, 100,  1),
		FireType(0xb,  0xe,  0x14, 0,  1,  0x50,  0x7, 0, 0xc, 1000, 0),
		FireType(0xc,  0x5,  0x5,  0,  1,  0x0,   0x0, 0, 0x1, 1000, 0),
		FireType(0xd,  0xf,  0x14, 0,  10, 0x50,  0x7, 0, 0xc, 1000, 0),
		FireType(0xe,  0xfa, 0xfa, 4,  1,  0x9c4, 0x4, 1, 0x3, 1000, 1),
		FireType(0xf,  0x32, 0x4b, 4,  1,  0x2ee, 0x4, 0, 0x3, 1000, 1),
		FireType(0x10, 0x4b, 0x64, 3,  1,  0x1f4, 0x4, 0, 0x3, 1000, 1),
		FireType(0x11, 0x7d, 0x96, 0,  1,  0x4e2, 0x4, 0, 0x3, 1000, 1),
		FireType(0x12, 0x14, 0x14, 0,  1,  0x0,   0x0, 0, 0x3, 1000, 1),
		FireType(0x13, 0x5,  0x5,  0,  1,  0x0,   0x0, 0, 0x3, 1000, 1),
		FireType(0x14, 0x64, 0x7d, 0,  1,  0x384, 0x4, 0, 0x3, 1000, 1),
		FireType(0x15, 0x1e, 0x32, 0,  1,  0x15e, 0x7, 0, 0x3, 1000, 1),
		FireType(0x16, 0x2d, 0x50, 7,  1,  0x0,   0x0, 1, 0x3, 1000, 1)
	};
	if (type >= ARRAYSIZE(FIRE_TYPE_TABLE_REG))
		return nullptr;
	return &FIRE_TYPE_TABLE_REG[type];
}

const FireType *FireTypeTable::get(uint16 type) {
	if (GAME_IS_REMORSE) {
		return _getFireTypeRemorse(type);
	} else {
		return _getFireTypeRegret(type);
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
