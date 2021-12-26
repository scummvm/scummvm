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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_INPUT_H
#define SAGA2_INPUT_H

#include "saga2/rect.h"

namespace Saga2 {

enum keyQualifiers {
	qualifierShift  = (1 << 0),
	qualifierControl = (1 << 1),
	qualifierAlt    = (1 << 2),

	// special qualifiers used internally
	spQualifierFunc = (1 << 7)              // indicates function key
};

const int           homeKey     = (0x47 << 8),
                    endKey      = (0x4F << 8),

                    upArrowKey  = (0x48 << 8),
                    leftArrowKey = (0x4B << 8),
                    rightArrowKey = (0x4D << 8),
                    downArrowKey = (0x50 << 8),

                    pageUpKey   = (0x49 << 8),
                    pageDownKey = (0x51 << 8),
                    insertKey   = (0x52 << 8),
                    deleteKey   = (0x53 << 8),
                    funcKey1    = (0x3B << 8),
                    funcKey2    = (0x3C << 8),
                    funcKey3    = (0x3D << 8),
                    funcKey4    = (0x3E << 8),
                    funcKey5    = (0x3F << 8),
                    funcKey6    = (0x40 << 8),
                    funcKey7    = (0x41 << 8),
                    funcKey8    = (0x42 << 8),
                    funcKey9    = (0x43 << 8),
                    funcKey10   = (0x44 << 8),
                    funcKey11   = (0x85 << 8),
                    funcKey12   = (0x86 << 8),

                    shFuncKey1  = (0x54 << 8),
                    shFuncKey2  = (0x55 << 8),
                    shFuncKey3  = (0x56 << 8),
                    shFuncKey4  = (0x57 << 8),
                    shFuncKey5  = (0x58 << 8),
                    shFuncKey6  = (0x59 << 8),
                    shFuncKey7  = (0x5A << 8),
                    shFuncKey8  = (0x5B << 8),
                    shFuncKey9  = (0x5C << 8),
                    shFuncKey10 = (0x5D << 8),
                    shFuncKey11 = (0x87 << 8),
                    shFuncKey12 = (0x88 << 8),

                    altFuncKey1 = (0x68 << 8),
                    altFuncKey2 = (0x69 << 8),
                    altFuncKey3 = (0x6A << 8),
                    altFuncKey4 = (0x6B << 8),
                    altFuncKey5 = (0x6C << 8),
                    altFuncKey6 = (0x6D << 8),
                    altFuncKey7 = (0x6E << 8),
                    altFuncKey8 = (0x6F << 8),
                    altFuncKey9 = (0x70 << 8),
                    altFuncKey10 = (0x71 << 8),
                    altFuncKey11 = (0x8B << 8),
                    altFuncKey12 = (0x8C << 8),

                    ctlFuncKey1 = (0x5E << 8),
                    ctlFuncKey2 = (0x5F << 8),
                    ctlFuncKey3 = (0x60 << 8),
                    ctlFuncKey4 = (0x61 << 8),
                    ctlFuncKey5 = (0x62 << 8),
                    ctlFuncKey6 = (0x63 << 8),
                    ctlFuncKey7 = (0x64 << 8),
                    ctlFuncKey8 = (0x65 << 8),
                    ctlFuncKey9 = (0x66 << 8),
                    ctlFuncKey10 = (0x67 << 8),
                    ctlFuncKey11 = (0x89 << 8),
                    ctlFuncKey12 = (0x8A << 8);

struct gMouseState {
	Point16         pos;
	uint8           right,
	                left;

	gMouseState() {
		right = 0;
		left = 0;
	}
};

} // end of namespace Saga2

#endif
