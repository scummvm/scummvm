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
 * $URL$
 * $Id$
 */


#ifndef TEENAGENT_OBJECTS_H
#define TEENAGENT_OBJECTS_H

#include "common/rect.h"
#include "graphics/surface.h"

namespace TeenAgent {

//move into separate header:

#include "common/pack-start.h"	// START STRUCT PACKING

struct Rect {
	uint16 left, top, right, bottom;

	inline Rect() : left(0), top(0), right(0), bottom(0) {}
	inline Rect(uint16 l, uint16 t, uint16 r, uint16 b) : left(l), top(t), right(r), bottom(b) {}

	inline bool in(const Common::Point &point) const {
		return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
	}
	inline Common::Point center() const {
		return Common::Point((right + left) / 2, (bottom + top) / 2);
	}
	inline bool valid() const {
		return left < 320 && right < 320 && top < 200 && bottom < 200;
	}
	void render(Graphics::Surface *surface, uint8 color) const;
	void dump() {
		debug(0, "rect[%u, %u, %u, %u]", left, top, right, bottom);
	}

	void clear() {
		left = top = right = bottom = 0;
	}
} PACKED_STRUCT;

struct Object {
	byte id;
	Rect rect;
	Rect actor_rect;
	byte actor_orientation;
	byte enabled;
	char name[1];

	enum {kActorUp = 1, kActorRight = 2, kActorDown = 3, kActorLeft = 4 };

	void dump();
	static Common::String description(const char *name);
	Common::String description() const;
} PACKED_STRUCT;

struct InventoryObject {
	byte id;
	byte animated;
	char name[1];
	Common::String description() const;
} PACKED_STRUCT;

struct UseObject {
	byte inventory_id;
	byte object_id;
	byte unk02;
	uint16 x, y;
	uint16 callback;
} PACKED_STRUCT;

struct Walkbox {
	byte unk00;
	byte orientation;
	Rect rect;
	byte unk0a;
	byte unk0b;
	byte unk0c;
	byte unk0d;
	void dump();
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

} // End of namespace TeenAgent

#endif
