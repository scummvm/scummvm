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
	INTRO,AIRPORT,TAXISTAND,STREET,GAMES,CABIN,KIOSK,
	KPALAST,CASHBOX,CITY1,CITY2,ELEVATOR,APARTMENT,SHIP,

	PYRAMID,PYRGANG,UPSTAIRS1,DOWNSTAIRS1,
	BOTTOM_RIGHT_DOOR,BOTTOM_LEFT_DOOR,UPSTAIRS2,DOWNSTAIRS2,
	UPPER_DOOR,PUZZLE_FRONT,PUZZLE_BEHIND,
	FORMULA1_F,FORMULA1_N,FORMULA2_F,FORMULA2_N,TOMATO,TOMATO,
	MONSTER_F,MONSTER1_N,MONSTER2_N,UPSTAIRS3,DOWNSTAIRS3,
	LGANG1,LGANG2,HOLE_ROOM,IN_HOLE,BODENTUER,BODENTUER_U,
	BST_DOOR,HALL,COFFIN_ROOM,MASK,

	MUSEUM,MUS_EING,MUS1,MUS2,MUS3,MUS4,MUS5,MUS6,MUS7,MUS8,
	MUS9,MUS10,MUS11,MUS_RUND,
	MUS12,MUS13,MUS14,MUS15,MUS16,MUS17,MUS18,MUS19,MUS20,MUS21,MUS22,
	NULLROOM
};

enum ObjectId {
	INVALIDOBJECT = -1,
	TAXI=1,WALLET,TRANSMITTER,KNIFE,POLE,BES_CABIN,MONEY,
	SLOT1,CHAIR,GANG,G_RIGHT,G_LEFT,PYRA_ENTRANCE,DOOR,BUTTON,
	PART0,PART1,PART2,PART3,PART4,PART5,PART6,PART7,
	PART8,PART9,PART10,PART11,PART12,PART13,PART14,PART15,
	TKNIFE,ROPE,NOTE,MOUTH,
	HOLE1,HOLE2,HOLE3,HOLE4,HOLE5,HOLE6,HOLE7,HOLE8,HOLE9,HOLE10,
	HOLE11,HOLE12,HOLE13,HOLE14,HOLE15,HOLE16,HOLE17,HOLE18,HOLE19,HOLE20,
	HOLE21,HOLE22,HOLE23,HOLE24,HOLE25,SHIELD,
	SLOT,HOLE,STONES,
	BST1,BST2,BST3,BST4,BST5,BST6,BST7,BST8,
	BST9,BST10,BST11,BST12,BST13,BST14,BST15,BST16,
	COFFIN,SUN,MONSTER,EYE,EYE1,EYE2,L_BALL,R_BALL,
	PRIZE,REAR_STREET,
	BMASK,BOTTLE,PLAYER,TOOTHBRUSH,BOOKS,LEXIcON,PLANT,SNAKE,
	CUP,JOYSTICK,BOTTLE1,BOTTLE2,BOTTLE3,BOTTLE4,BOX,FACES,
	SELLER,POSTER,AXACUSSER,KP_ENTRANCE,CHIP,CARD,
	DOODLE,DOODLE2,BELL,GROPE,DOOR_L,DOOR_R,ID_CARD,
	MAGNET,UNDER_BED,KEY,VALVE,WARDROBE,DISCMAN,
	SMALL_DOOR,BACK_MONEY,WALL,MAMMAL,IMITATION,SP_KEYCARD,ALARM_CRACKER,
	ENCRYPTED_DOOR,ALARM_ANLAGE,MUS_STREET,BIG_DOOR,INVESTMENT,
	HANDLE,COUNTER,DOOR_SWITCH,SUIT,MANAGEMENT,RMANAGEMENT,
	MUSCARD,SKOPF
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
