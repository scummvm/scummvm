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

#ifndef IMMORTAL_STORY_H
#define IMMORTAL_STORY_H

namespace Immortal {

enum DoorDir : bool {
	kLeft = false,
	kRight = true
};

enum RoomFlag : uint8 {
	kRoomFlag0 = 0x1,
	kRoomFlag1 = 0x2,
	kRoomFlag2 = 0x4,
	kRoomFlag3 = 0x8
};

enum FPattern {
	kFlameNormal,
	kFlameCandle,
	kFlameOff,
	kFlameGusty
};

enum OPMask : uint8 {
	kOPMaskRoom,
	kOPMaskInRoom,
	kOPMaskFlame,
	kOPMaskUnivAt,
	kOPMaskMonster,
	kOPMaskDoor,
	kOPMaskObject,
	kOPMaskRecord
};

enum ObjFlag : uint8 {
	kObjUsesFireButton = 0x40,
	kObjIsInvisible    = 0x20,
	kObjIsRunning      = 0x10,
	kObjIsChest        = 0x08,
	kObjIsOnGround     = 0x04,
	kObjIsF1           = 0x02,
	kObjIsF2           = 0x01
};

enum MonsterFlag : uint8 {
	kMonstIsTough  = 0x10,
	kMonstIsDead   = 0x20,
	kMonstIsPoss   = 0x40,
	kMonstIsBaby   = 0x40,
	kMonstIsEngage = 0x80
};

enum IsA : uint8 {
	kIsAF1 = 0x20,
	kIsAF2 = 0x40
};

enum Program {						// This will likely be moved to a monster ai specific file later
};

enum Str {
	kStrOldGame,
	kStrEnterCertificate,
	kStrBadCertificate,
	kStrCertificate,
	kStrCertificate2,
	kStrTitle0,
	kStrTitle4,
	kStrGold,
	kStrYouWin,
	kStrGameOver,
};

struct Pickup {
	//pointer to function
	int _param;
};

struct Use {
	//pointer to function
	int _param;
};

struct ObjType {
	Str _str;
	Str _desc;
	int _size;
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
	uint8 _x;
	uint8 _y;
 RoomFlag _flags;
 	SRoom() {}
 	SRoom(uint8 x, uint8 y, RoomFlag f) {
 			_x = x;
 			_y = y;
 		_flags = f;
 	}
};

struct SDoor {
  DoorDir _dir;
	uint8 _x;
	uint8 _y;
	uint8 _fromRoom;
	uint8 _toRoom;
	 bool _isLocked;
	 SDoor() {}
	 SDoor(DoorDir d, uint8 x, uint8 y, uint8 f, uint8 t, bool l) {
	 		  _dir = d;
	 			_x = x;
	 			_y = y;
	 	 _fromRoom = f;
	 	   _toRoom = t;
	 	 _isLocked = l;
	 }
};

struct SFlame {
	uint8 _x;
	uint8 _y;
 FPattern _pattern;
 	SFlame() {}
 	SFlame(uint8 x, uint8 y, FPattern p) {
 			  _x = x;
 			  _y = y;
 		_pattern = p;
 	}
};

struct UnivAt {
	uint8 _initialUnivX;
	uint8 _initialUnivY;
	uint8 _playerPointX;
	uint8 _playerPointY;
	int *_ladders;
	UnivAt() {}
	UnivAt(uint8 iX, uint8 iY, uint8 pX, uint8 pY, int l[]) {
		_initialUnivX = iX;
		_initialUnivY = iY;
		_playerPointX = pX;
		_playerPointY = pY;
		_ladders = l;
	}
};

struct SObj {
	  uint8 _x;
	  uint8 _y;
	ObjType _type;
	ObjFlag _flags;
	  uint8 _tmp;
SpriteFrame _frame;
	SObj() {}
	SObj(uint8 x, uint8 y, ObjType t, ObjFlag f, uint8 tmp, SpriteFrame s) {
 		    _x = x;
 		    _y = y;
 		 _type = t;
 		_flags = f;
 		  _tmp = tmp;
 		_frame = s;
	}
};

struct SMonster {
	  uint8 _x;
	  uint8 _y;
	  uint8 _hits;
	  uint8 _madAt;
		IsA _isA;
    Program _program;
 SpriteName _sprite;
MonsterFlag _flags;
	SMonster() {}
	SMonster(uint8 x, uint8 y, uint8 h, uint8 m, IsA i, Program p, SpriteName s, MonsterFlag mf) {
 		  _x = x;
 		  _y = y;
 	   _hits = h;
 	  _madAt = m;
 		_isA = i;
 	_program = p;
 	 _sprite = s;
 	  _flags = mf;
	}
};

struct Story {
	 int _levelNum;
	 int _partNum;
  UnivAt _UnivAt;
   SRoom *_rooms;
   SDoor *_doors;
  SFlame **_flames;
    SObj **_objects;
SMonster **_monsters;
};

} // namespace immortal

#endif











