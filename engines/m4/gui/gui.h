
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
#include "m4/graphics/gr_buff.h"

namespace M4 {

struct M4sprite {
	M4sprite *next = nullptr;
	M4sprite *prev = nullptr;
	int32 x = 0;
	int32 y = 0;
	int32 w = 0;
	int32 h = 0;
	int32 xOffset = 0;	// the "hotspot" of the sprite, ie: the registration point
	int32 yOffset = 0;
	uint8 encoding = 0;
	uint8 *data = nullptr;
	MemHandle sourceHandle = nullptr;
	int32 sourceOffset = 0;
};

struct transSprite {
	M4sprite *srcSprite = nullptr;
	Buffer *scrnBuffer = nullptr;
};


struct M4Rect {
	int32 x1, y1, x2, y2;
};

struct RectList {
	RectList *next;
	RectList *prev;
	int32 x1, y1, x2, y2;
};

struct matte {
	matte *nextMatte;
	void *myScreen;
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;
	int32 w;
	int32 h;
	uint8 *SrcBuffer;
	uint32 SrcPitch;
};

} // End of namespace M4

#endif
