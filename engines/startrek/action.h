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

#ifndef STARTREK_ACTION_H
#define STARTREK_ACTION_H

#include "common/serializer.h"

namespace StarTrek {

class Room;

enum ActionTypes {
	ACTION_TICK = 0,

	ACTION_WALK = 1, // Actions 1-5 are directly usable on away missions.
	ACTION_USE = 2,
	ACTION_GET = 3,
	ACTION_LOOK = 4,
	ACTION_TALK = 5,

	ACTION_TOUCHED_WARP = 6,
	ACTION_TOUCHED_HOTSPOT = 7, // Second kind of "hotspot" only relevant when an object touches them
	ACTION_TIMER_EXPIRED = 8,

	ACTION_DONE_ANIM = 10,
	ACTION_DONE_WALK = 12,

	// TODO: remove these as redundant. They only still exist so I don't need to redo the
	// manual spacing in the room action lists.
	ACTION_FINISHED_ANIMATION = 10,
	ACTION_FINISHED_WALKING = 12,

	ACTION_OPTIONS = 13, // Not really an action, but selectable from action menu

	ACTION_LIST_END = -1
};

struct Action {
	int8 type;
	byte b1;
	byte b2;
	byte b3;

	// ACTION_USE, ACTION_GET, ACTION_LOOK, ACTION_TALK
	byte activeObject() const {
		return b1;
	}
	byte passiveObject() const {
		return b2;
	}

	bool operator==(const Action &a) const {
		return type == a.type && b1 == a.b1 && b2 == a.b2 && b3 == a.b3;
	}

	uint32 getBitmask() const {
		uint32 ret = 0;
		if (type != -1)
			ret |= (0xff << 24);
		if (b1 != 0xff)
			ret |= (0xff << 16);
		if (b2 != 0xff)
			ret |= (0xff << 8);
		if (b3 != 0xff)
			ret |= (0xff << 0);
		return ret;
	}

	uint32 toUint32() const {
		return (type << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0);
	}

	void saveLoadWithSerializer(Common::Serializer &ser) {
		ser.syncAsByte(type);
		ser.syncAsByte(b1);
		ser.syncAsByte(b2);
		ser.syncAsByte(b3);
	}
};

} // End of namespace StarTrek

#endif
