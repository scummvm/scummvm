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

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

void lua_PatchGrimSave() {
	// Since ResidualVM 0.2.0, a ResidualVM/ScummVM specific patch was provided broken.
	// We patch here the code to fix all saves containing this invalid code.
	// cf. bug #13139 and #14987
	// The patch here doesn't match exactly the fixed patch.
	// We minimize here the number of bytes to patch with an equivalent result.
	TProtoFunc *tempProtoFunc = (TProtoFunc *)rootproto.next;
	while (tempProtoFunc) {
		if ((tempProtoFunc->lineDefined == 77) &&
			(strcmp(tempProtoFunc->fileName->str, "Scripts\\vd.lua") == 0) &&
			(memcmp(tempProtoFunc->code + 210, "\x22\x29\x39\x03\x32\x22\x30\x79\x02", 9) == 0)) {
			tempProtoFunc->code[211] = 0x33;
			tempProtoFunc->code[218] = 0x03;
			break;
		}
		tempProtoFunc = (TProtoFunc *)tempProtoFunc->head.next;
	}
}

} // end of namespace Grim
