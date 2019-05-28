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
 */

#ifndef SUPERNOVA2_MS2_DEF_H
#define SUPERNOVA2_MS2_DEF_H

#include "common/scummsys.h"

namespace Supernova2 {

const int kTextSpeed[] = {19, 14, 10, 7, 4};
const int kMsecPerTick = 55;

const int kMaxSection = 40;
const int kMaxDialog = 2;
const int kMaxObject = 25;
const int kMaxCarry = 30;

const int kSleepAutosaveSlot = 999;

const byte kShownFalse = 0;
const byte kShownTrue = 1;

enum MessagePosition {
	kMessageNormal,
	kMessageLeft,
	kMessageRight,
	kMessageCenter,
	kMessageTop
};

enum ObjectType {
	NULLTYPE    =      0,
	TAKE        =      1,
	OPENABLE    =      2,
	OPENED      =      4,
	CLOSED      =      8,
	EXIT        =     16,
	PRESS       =     32,
	COMBINABLE  =     64,
	CARRIED     =    128,
	UNNECESSARY =    256,
	WORN        =    512,
	TALK        =   1024,
	OCCUPIED    =   2048,
	CAUGHT      =   4096
};
typedef uint16 ObjectTypes;

enum Action {
	ACTION_WALK,
	ACTION_LOOK,
	ACTION_TAKE,
	ACTION_OPEN,
	ACTION_CLOSE,
	ACTION_PRESS,
	ACTION_PULL,
	ACTION_USE,
	ACTION_TALK,
	ACTION_GIVE
};

enum RoomId {
	INTRO, OUTRO,
	NULLROOM
};

enum ObjectId {
	INVALIDOBJECT = -1,
	NULLOBJECT = 0
};

enum StringId {
	kNoString = -1, kStringDefaultDescription
};

ObjectType operator|(ObjectType a, ObjectType b);
ObjectType operator&(ObjectType a, ObjectType b);
ObjectType operator^(ObjectType a, ObjectType b);
ObjectType &operator|=(ObjectType &a, ObjectType b);
ObjectType &operator&=(ObjectType &a, ObjectType b);
ObjectType &operator^=(ObjectType &a, ObjectType b);

struct Object {
	Object()
		: _name(kNoString)
		, _description(kStringDefaultDescription)
		, _id(INVALIDOBJECT)
		, _roomId(NULLROOM)
		, _type(NULLTYPE)
		, _click(0)
		, _click2(0)
		, _section(0)
		, _exitRoom(NULLROOM)
		, _direction(0)
	{}
	Object(byte roomId, StringId name, StringId description, ObjectId id, ObjectType type,
		   byte click, byte click2, byte section = 0, RoomId exitRoom = NULLROOM, byte direction = 0)
		: _name(name)
		, _description(description)
		, _id(id)
		, _roomId(roomId)
		, _type(type)
		, _click(click)
		, _click2(click2)
		, _section(section)
		, _exitRoom(exitRoom)
		, _direction(direction)
	{}

	void resetProperty(ObjectType type = NULLTYPE) {
		_type = type;
	}

	void setProperty(ObjectType type) {
		_type |= type;
	}

	void disableProperty(ObjectType type) {
		_type &= ~type;
	}

	bool hasProperty(ObjectType type) const {
		return _type & type;
	}

	static bool combine(Object &obj1, Object &obj2, ObjectId id1, ObjectId id2) {
		if (obj1.hasProperty(COMBINABLE))
			return (((obj1._id == id1) && (obj2._id == id2)) ||
					((obj1._id == id2) && (obj2._id == id1)));
		else
			return false;
	}

	byte _roomId;
	StringId _name;
	StringId _description;
	ObjectId _id;
	ObjectTypes _type;
	byte _click;
	byte _click2;
	byte _section;
	RoomId _exitRoom;
	byte _direction;
};

#define ticksToMsec(x) (x * kMsecPerTick)

}

#endif // SUPERNOVA2_MS2_DEF_H
