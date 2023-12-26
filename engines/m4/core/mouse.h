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

#ifndef M4_CORE_MOUSE_H
#define M4_CORE_MOUSE_H

#include "m4/m4_types.h"

namespace M4 {

enum cursor_states {
	kARROW, kLOOK, kTAKE, kUSE
};

struct MouseDriverInfo {
	byte MajorVersion = 0;
	byte MinorVersion = 0;
	byte MouseType = 0;
	byte MouseIRQ = 0;
};

int mouse_init();
void mouse_uninstall();
byte *GetMousePicture();

} // namespace M4

#endif
