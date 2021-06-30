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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/input.h"

namespace Saga2 {

errorCode InitMouse(int16 width, int16 height) {
	warning("STUB: InitMouse()");
	return errUnknown;
}

void CleanupMouse(void) {
	warning("STUB: CleanupMouse()");
}

gTimeStamp ReadTimer() {
	warning("STUB: ReadTimer()");
	return 0;
}

bool ReadKeyboard(int &key, int &qual) {
	warning("STUB: ReadKeyboard()");
	return false;
}

} // end of namespace Saga2
