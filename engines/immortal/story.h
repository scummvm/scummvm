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

// Definitions are the enum for the set of global definitions in Story.GS
#include "immortal/definitions.h"

// Sprite List is a list of all sprite definitions (could be included in definitions.h, but sprite_list.gs was a separate source file and is sprite specific)
#include "immortal/sprite_list.h"

#ifndef IMMORTAL_STORY_H
#define IMMORTAL_STORY_H

namespace Immortal {

struct Frame;
struct DataSprite;
struct Sprite;

// We need a few two-dimentional vectors, and writing them out in full each time is tedious
template<class T> using CArray2D = Common::Array<Common::Array<T>>;

// These maximum numbers aren't really needed, because most of these are vectors and have .size()
enum StoryMaxes {
	kMaxRooms		 = 16,
	kMaxDoors		 = 10,
	kMaxFlames 	     = 32,
	kMaxFlamesInRoom = 5,
	kMaxObjects 	 = 42,
	kMaxMonsters 	 = 20,
	kMaxGenSprites	 = 6,
	kMaxCycles		 = 32
};

// These are flags that are relevant to their specific story data structures
enum RoomFlag : uint8 {							// Generic properties available to each room
	kRoomFlag0 = 0x01,
	kRoomFlag1 = 0x02,
	kRoomFlag2 = 0x04,
	kRoomFlag3 = 0x08
};

enum ObjFlag : uint8 {							// Properties of the object essentially
	kObjUsesFireButton = 0x40,
	kObjIsInvisible    = 0x20,
	kObjIsRunning      = 0x10,
	kObjIsChest        = 0x08,
	kObjIsOnGround     = 0x04,
	kObjIsF1           = 0x02,
	kObjIsF2           = 0x01,
	kObjNone		   = 0x0
};

enum IsA : uint8 {								// To be completely honest, I'm not really sure what this is. It seems to be more object flags, but they act a little strangely
	kIsAF1 	 = 0x20,
	kIsAF2 	 = 0x40,
	kIsANone = 0x0,
};

enum MonsterFlag : uint8 {						// Mostly properties of the AI for a given monster, *including the player*
	kMonstIsNone   = 0x00,
	kMonstIsTough  = 0x10,
	kMonstIsDead   = 0x20,
	kMonstIsPoss   = 0x40,
	kMonstIsBaby   = 0x40,
	kMonstIsEngage = 0x80,
	kMonstPlayer   = 0x00,
	kMonstMonster  = 0x01,
	kMonstAnybody  = 0x02,
	kMonstNobody   = 0x03,
	kMonstA 	   = 0x04,
	kMonstB 	   = 0x05,
	kMonstC 	   = 0x06,
	kMonstD 	   = 0x07
};

// Flame pattern is used by the story data, in-room data, *and* the level based total flame data. So it needs to be in story.h to be used by immortal.h and room.h
enum FPattern : uint8 {							// This defines which Cyc animation it uses
	kFlameNormal,
	kFlameCandle,
	kFlameOff,
	kFlameGusty
};

// Cycles define the animation of sprites within a level. There is a fixed total of cycles available, and they are not room dependant
struct Cycle {
DataSprite *_dSprite;
	   int  _numCycles;
	   int *_frames;
};

// Object Pickup defines how an object can be picked up by the player, with different functions
enum SObjPickup {
};

struct Pickup {
	//pointer to function
	int _param;
};

// Iirc damage is used by object types as well as enemy types
enum SDamage {
};

struct Damage {
};

// Use is self explanitory, it defines the function and parameters for using an object
enum SObjUse {
};

struct Use {
	//pointer to function
	int _param;
};

struct ObjType {
	Str _str = kStrNoDesc;
	Str _desc = kStrNoDesc;
	int _size = 0;
 Pickup _pickup;
	Use _use;
	Use _run;
};

/* Strictly speaking, many of these structs (which were rom data written dynamically
 * with compiler macros) combine multiple properties into single bytes (ex. room uses
 * bits 0-2 of X to also hold the roomOP, and bits 0-2 of Y to hold flags). However
 * for the moment there's no need to replicate this particular bit of space saving.
 */
struct SRoom {
	uint8 _x = 0;
	uint8 _y = 0;
 RoomFlag _flags = kRoomFlag0;
 	SRoom() {}
 	SRoom(uint8 x, uint8 y, RoomFlag f) {
 			_x = x;
 			_y = y;
 		_flags = f;
 	}
};

struct SDoor {
  uint8 _dir = 0;
	uint8 _x   = 0;
	uint8 _y   = 0;
	uint8 _fromRoom = 0;
	uint8 _toRoom   = 0;
	 bool _isLocked = false;
	 SDoor() {}
	 SDoor(uint8 d, uint8 x, uint8 y, uint8 f, uint8 t, bool l) {
	 	_dir = d;
	 	_x = x;
	 	_y = y;
	 	_fromRoom = f;
	 	_toRoom = t;
		_isLocked = l;
	 }
};

struct SFlame {
	 uint8 _x = 0;
	 uint8 _y = 0;
FPattern _p = kFlameOff;
	SFlame() {}
 	SFlame(uint8 x, uint8 y, FPattern p) {
 		_x = x;
		_y = y;
 		_p = p;
 	}
};

struct SObj {
	  uint8 _x = 0;
	  uint8 _y = 0;
   SObjType _type = kTypeTrap;
	  uint8 _flags = 0;
SpriteFrame _frame = kNoFrame;
Common::Array<uint8> _traps;
	SObj() {}
	SObj(uint8 x, uint8 y, SObjType t, SpriteFrame s, uint8 f, Common::Array<uint8> traps) {
 		    _x = x;
 		    _y = y;
 		 _type = t;
 		_flags = f;
 		_traps = traps;
 		_frame = s;
	}
};

struct SMonster {
	    uint8 _x = 0;
	    uint8 _y = 0;
	    uint8 _hits = 0;
MonsterFlag _madAt = kMonstIsNone;
	    uint8 _flags = 0;
 SpriteName _sprite = kCandle;
Common::Array<Motive> _program;
	SMonster() {}
	SMonster(uint8 x, uint8 y, uint8 h, MonsterFlag m, uint8 f, Common::Array<Motive> p, SpriteName s) {
 		    _x = x;
 		    _y = y;
 	     _hits = h;
 	    _madAt = m;
 	    _flags = f;
 	  _program = p;
 	   _sprite = s;
	}
};

struct Story {
	 int _level = 0;
	 int _part  = 1;

   uint8 _initialUnivX = 0;
   uint8 _initialUnivY = 0;
   uint8 _playerPointX = 0;
   uint8 _playerPointY = 0;

  Common::Array<int> _ladders;
Common::Array<SRoom> _rooms;
Common::Array<SDoor> _doors;
    CArray2D<SFlame> _flames;
      CArray2D<SObj> _objects;
  CArray2D<SMonster> _monsters;
};

} // namespace immortal

#endif











