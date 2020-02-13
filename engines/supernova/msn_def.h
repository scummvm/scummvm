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

#ifndef SUPERNOVA_MSN_DEF_H
#define SUPERNOVA_MSN_DEF_H

// Avoid error on expansion of macro switch in the ObjectID enum below
#ifdef SWITCH
#undef SWITCH
#endif

#include "common/scummsys.h"

namespace Supernova {

const int kTextSpeed[] = {19, 14, 10, 7, 4};
const int kMsecPerTick = 55;

const int kMaxSection = 40;
const int kMaxDialog = 2;
const int kMaxObject = 30;
const int kMaxCarry = 30;

const int kSleepAutosaveSlot = 999;

const byte kShownFalse = 0;
const byte kShownTrue = 1;

const byte kSectionInvert = 128;

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
	// Supernova 1
	INTRO1,CORRIDOR_ROOM,HALL,SLEEP,COCKPIT,AIRLOCK,
	HOLD,LANDINGMODULE,GENERATOR,OUTSIDE,
	CABIN_R1,CABIN_R2,CABIN_R3,CABIN_L1,CABIN_L2,CABIN_L3,BATHROOM,

	ROCKS,CAVE,MEETUP,ENTRANCE,REST,ROGER,GLIDER,MEETUP2,MEETUP3,

	CELL,CORRIDOR1,CORRIDOR2,CORRIDOR3,CORRIDOR4,CORRIDOR5,CORRIDOR6,CORRIDOR7,CORRIDOR8,CORRIDOR9,
	BCORRIDOR,GUARD,GUARD3,OFFICE_L1,OFFICE_L2,OFFICE_R1,OFFICE_R2,OFFICE_L,
	ELEVATOR,STATION,SIGN_ROOM,OUTRO,NUMROOMS1,

	// Supernova 2
	INTRO2 = 0,AIRPORT,TAXISTAND,STREET,GAMES,CABIN2,KIOSK,
	CULTURE_PALACE,CHECKOUT,CITY1,CITY2,ELEVATOR2,APARTMENT,SHIP,

	PYRAMID,PYR_ENTRANCE,UPSTAIRS1,DOWNSTAIRS1,
	BOTTOM_RIGHT_DOOR,BOTTOM_LEFT_DOOR,UPSTAIRS2,DOWNSTAIRS2,
	UPPER_DOOR,PUZZLE_FRONT,PUZZLE_BEHIND,
	FORMULA1_F,FORMULA1_N,FORMULA2_F,FORMULA2_N,TOMATO_F,TOMATO_N,
	MONSTER_F,MONSTER1_N,MONSTER2_N,UPSTAIRS3,DOWNSTAIRS3,
	LCORRIDOR1,LCORRIDOR2,HOLE_ROOM,IN_HOLE,FLOORDOOR,FLOORDOOR_U,
	BST_DOOR,HALL2,COFFIN_ROOM,MASK,

	MUSEUM,MUS_ENTRANCE,MUS1,MUS2,MUS3,MUS4,MUS5,MUS6,MUS7,MUS8,
	MUS9,MUS10,MUS11,MUS_ROUND,
	MUS12,MUS13,MUS14,MUS15,MUS16,MUS17,MUS18,MUS19,MUS20,MUS21,MUS22,
	NUMROOMS2,NULLROOM
};

enum ObjectId {
	INVALIDOBJECT = -1,
	NULLOBJECT = 0,

	// Supernova 1
	KEYCARD,KNIFE,WATCH,
	SOCKET,
	BUTTON,HATCH1,
	BUTTON1,BUTTON2,MANOMETER,SUIT,HELMET,LIFESUPPORT,
	SCRAP_LK,OUTERHATCH_TOP,GENERATOR_TOP,TERMINALSTRIP,LANDINGMOD_OUTERHATCH,
	HOLD_WIRE,
	LANDINGMOD_BUTTON,LANDINGMOD_SOCKET,LANDINGMOD_WIRE,LANDINGMOD_HATCH,LANDINGMOD_MONITOR,
	KEYBOARD,
	KEYCARD2,OUTERHATCH,GENERATOR_WIRE,TRAP,SHORT_WIRE,CLIP,
	VOLTMETER,LADDER,GENERATOR_ROPE,
	KITCHEN_HATCH,SLEEP_SLOT,
	MONITOR,INSTRUMENTS,
	COMPUTER,CABINS,CABIN,
	SLOT_K1,SLOT_K2,SLOT_K3,SLOT_K4,
	SHELF1,SHELF2,SHELF3,SHELF4,
	ROPE,BOOK,DISCMAN,CHESS,
	SLOT_KL1,SLOT_KL2,SLOT_KL3,SLOT_KL4,
	SHELF_L1,SHELF_L2,SHELF_L3,SHELF_L4,
	PISTOL,BOOK2,SPOOL,
	RECORD,TURNTABLE,TURNTABLE_BUTTON,WIRE,WIRE2,PLUG,
	PEN,
	BATHROOM_DOOR,BATHROOM_EXIT,SHOWER,TOILET,

	STONE,
	SPACESHIPS,SPACESHIP,STAR,DOOR,MEETUP_SIGN,
	PORTER,BATHROOM_BUTTON,BATHROOM_SIGN,KITCHEN_SIGN,CAR_SLOT,
	ARSANO_BATHROOM,COINS,SCHNUCK,EGG,PILL,PILL_HULL,STAIRCASE,
	MEETUP_EXIT,
	ROGER_W,WALLET,KEYCARD_R,CUP,
	GLIDER_BUTTON1,GLIDER_BUTTON2,GLIDER_BUTTON3,GLIDER_BUTTON4,GLIDER_SLOT,GLIDER_BUTTONS,
	GLIDER_DISPLAY,GLIDER_INSTRUMENTS,GLIDER_KEYCARD,
	UFO,

	CELL_BUTTON,CELL_TABLE,CELL_WIRE,TRAY,CELL_DOOR,MAGNET,
	NEWSPAPER,TABLE,
	PILLAR1,PILLAR2,DOOR1,DOOR2,DOOR3,DOOR4,
	GUARDIAN,LAMP,
	MASTERKEYCARD,PAINTING,MONEY,LOCKER,LETTER,
	JUNGLE,STATION_SLOT,STATION_SIGN,


	// Supernova2
	TAXI,TRANSMITTER,ROD,OCCUPIED_CABIN,
	SLOT1,CHAIR,CORRIDOR,G_RIGHT,G_LEFT,PYRA_ENTRANCE,
	PART0,PART1,PART2,PART3,PART4,PART5,PART6,PART7,
	PART8,PART9,PART10,PART11,PART12,PART13,PART14,PART15,
	TKNIFE,NOTE,MOUTH,
	HOLE1,HOLE2,HOLE3,HOLE4,HOLE5,HOLE6,HOLE7,HOLE8,HOLE9,HOLE10,
	HOLE11,HOLE12,HOLE13,HOLE14,HOLE15,HOLE16,HOLE17,HOLE18,HOLE19,HOLE20,
	HOLE21,HOLE22,HOLE23,HOLE24,HOLE25,SIGN,
	SLOT,HOLE,STONES,
	BST1,BST2,BST3,BST4,BST5,BST6,BST7,BST8,
	BST9,BST10,BST11,BST12,BST13,BST14,BST15,BST16,
	COFFIN,SUN,MONSTER,EYE,EYE1,EYE2,L_BALL,R_BALL,
	PRIZE,REAR_STREET,
	BMASK,BOTTLE,PLAYER,TOOTHBRUSH,BOOKS,LEXICON,PLANT,SNAKE,
	JOYSTICK,BOTTLE1,BOTTLE2,BOTTLE3,BOTTLE4,BOX,FACES,
	SELLER,POSTER,AXACUSSER,KP_ENTRANCE,CHIP,CARD,
	SCRIBBLE1,SCRIBBLE2,BELL,KEYPAD,DOOR_L,DOOR_R,ID_CARD,
	UNDER_BED,KEY,HATCH,CABINET,
	SMALL_DOOR,BACK_MONEY,WALL,SUCTION_CUP,IMITATION,SP_KEYCARD,ALARM_CRACKER,
	ENCRYPTED_DOOR,ALARM_SYSTEM,MUS_STREET,BIG_DOOR,MUSIC_SYSTEM,
	HANDLE,SWITCH,DOOR_SWITCH,CABLE,RCABLE,
	MUSCARD,HEAD, DISPLAY
};

enum StringId {
	kNoString = -1,
	// 0
	kStringCommandGo = 0, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
	kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive,
	kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen, kStringStatusCommandClose,
	kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk, kStringStatusCommandGive,
	kPhrasalVerbParticleGiveTo, kPhrasalVerbParticleUseWith, kStringDefaultDescription, kStringDialogSeparator, kStringConversationEnd,
	// 25
	kStringHelpOverview1, kStringHelpOverview2, kStringHelpOverview3, kStringHelpOverview4, kStringHelpOverview5,
	kStringHelpOverview6, kStringHelpOverview7, kStringTextSpeed, kStringLeaveGame, kStringYes,
	kStringNo
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
	Object(byte roomId, int name, int description, ObjectId id, ObjectType type,
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
	int _name;
	int _description;
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

#endif // SUPERNOVA_MSN_DEF_H
