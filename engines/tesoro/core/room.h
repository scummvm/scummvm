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

#ifndef TESORO_CORE_ROOM_H
#define TESORO_CORE_ROOM_H

#include "tesoro/core/objectlist.h"

namespace Tesoro {

class Object;

/** One room of a level: the objects standing in it, kept ordered by their x position. */
class Room {
public:
	/**
	 * index: position in the level;
	 * sortOffset: draw order offset accumulated from the rooms before it.
	 * */
	void init(int32 index, int32 cameraDivisor, int16 sortOffset);

	/** Puts the object in front of the first object that is further left, after objects at the same x. */
	void insertSorted(Object *object);

	void removeObject(Object *object);

	int32 index() const { return _index; }
	/** Divisor the camera uses when it is shown for this room. */
	int32 cameraDivisor() const { return _cameraDivisor; }
	int16 sortOffset() const { return _sortOffset; }
	const ObjectList &objects() const { return _objects; }
	ObjectList &objects() { return _objects; }

private:
	int32 _index = 0;
	int32 _cameraDivisor = 0;	// dword from the room record
	int16 _sortOffset = 0;
	ObjectList _objects;
};

} // End of namespace Tesoro

#endif
