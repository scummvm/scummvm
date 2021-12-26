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

#ifndef AGS_SHARED_AC_INTERFACE_BUTTON_H
#define AGS_SHARED_AC_INTERFACE_BUTTON_H

#include "ags/shared/core/types.h"

namespace AGS3 {

#define MAXBUTTON       20
#define IBFLG_ENABLED   1
#define IBFLG_INVBOX    2

struct InterfaceButton {
	int x, y, pic, overpic, pushpic, leftclick;
	int rightclick; // if inv, then leftclick = wid, rightclick = hit
	int reserved_for_future;
	int8 flags;
	void set(int xx, int yy, int picc, int overpicc, int actionn);
};

} // namespace AGS3

#endif
