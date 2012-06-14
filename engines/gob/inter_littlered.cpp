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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gob/gob.h"
#include "gob/inter.h"

namespace Gob {

#define OPCODEVER Inter_LittleRed
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_LittleRed::Inter_LittleRed(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_LittleRed::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();
}

void Inter_LittleRed::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();
}

void Inter_LittleRed::setupOpcodesGob() {
	OPCODEGOB(500, o2_playProtracker);
	OPCODEGOB(501, o2_stopProtracker);
}

} // End of namespace Gob
