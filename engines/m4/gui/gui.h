
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

#ifndef M4_GUI_GUI_H
#define M4_GUI_GUI_H

#include "m4/m4_types.h"

namespace M4 {

struct M4sprite {
	struct M4sprite *next;
	struct M4sprite *prev;
	int32			 x;
	int32			 y;
	int32			 w;
	int32			 h;
	int32			 xOffset;	// the "hotspot" of the sprite, ie: the registration point
	int32			 yOffset;
	uint8			 encoding;
	uint8 *data;
	Handle			 sourceHandle;
	int32			 sourceOffset;
};

struct M4Rect {
	int32 x1, y1, x2, y2;
};

struct RectList {
	RectList *next;
	RectList *prev;
	int32 x1, y1, x2, y2;
};

} // End of namespace M4

#endif
