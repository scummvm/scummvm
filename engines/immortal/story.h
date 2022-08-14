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

#include "immortal/sprite_list.h"						// This is an enum of all available sprites

#ifndef IMMORTAL_STORY_H
#define IMMORTAL_STORY_H

namespace Immortal {

// We need a few two-dimentional vectors, and writing them out in full each time is tedious
template<class T> using CArray2D = Common::Array<Common::Array<T>>;

enum DoorDir : bool {
	kLeft = false,
	kRight = true
};

enum RoomFlag : uint8 {							// Generic properties available to each room
	kRoomFlag0 = 0x1,
	kRoomFlag1 = 0x2,
	kRoomFlag2 = 0x4,
	kRoomFlag3 = 0x8
};

enum FPattern : uint8 {							// This defines which Cyc animation it uses
	kFlameNormal,
	kFlameCandle,
	kFlameOff,
	kFlameGusty
};

enum OPMask : uint8 {							// These are not actually needed anymore, they were for the original compiler method for making story.gs. Keeping it just in case for now
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
	kObjIsF2           = 0x01,
	kObjNone		   		 = 0x0
};

enum MonsterFlag : uint8 {
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

enum IsA : uint8 {
	kIsAF1 = 0x20,
	kIsAF2 = 0x40,
	kIsANone = 0x0,
};

enum Motive {						// This will likely be moved to a monster ai specific file later
	kMotiveRoomCombat,
	kMotiveShadeFind,
	kMotiveShadeLoose,
	kMotiveEngage,
	kMotiveUpdateGoal,
	kMotiveFollow,
	kMotiveShadeHesitate,
	kMotiveEasyRoomCombat,
	kMotiveFind8,
	kMotiveLoose4,
	kMotiveDefensiveCombat,
	kMotiveUlinTalk,
	kMotiveGive,
	kMotiveUseUpMonster,
	kMotiveAliveRoomCombat,
	kMotiveFindAlways,
	kMotivePlayerCombat,
	kMotiveJoystick,
	kMotivePlayerDoor,
	kMotivewaittalk2,
	kMotiveGetDisturbed,
	kMotiveLoose32,
	kMotiveIfNot1Skip1,
};

enum Str {
	kStrNoDesc,
	kStrSword,
	kStrSwordDesc,
	kStrBonesText1,
	kStrBonesText2,
	kStrBonesText3,
	kStrComp,
	kStrCompDesc,
	kStrOpenBag,
	kStrThrowComp,
	kStrSmithText1,
	kStrSmithText2,
	kStrCarpet,
	kStrBomb,
	kStrBombDesc,
	kStrPickItUp,
	kStrYesNo,
	kStrOther,
	kStrChestKey,
	kStrDoorKey,
	kStrChestKeyDesc,
	kStrOpenChestDesc,
	kStrPutItOn,
	kStrDropItThen,
	kStrChestDesc,
	kStrGoodChestDesc,
	kStrBadChestDesc,
	kStrComboLock,
	kStrGold,
	kStrFindGold,
	kStrNull,
	kStrNotHere,
	kStrUnlockDoor,
	kStrWeak1,
	kStrDummyWater,
	kStrBadWizard,
	kStrDiesAnyway,
	kStrDoorKeyDesc,
	kStrNoteDesc,
	kStrNote,
	kStrLootBodyDesc,
	kStrNotEnough,
	kStrGameOver,
	kStrYouWin,
	kStrWormFoodDesc,
	kStrWormFood,
	kStrStoneDesc,
	kStrStone,
	kStrGemDesc,
	kStrGem,
	kStrFireBallDesc,
	kStrFireBall,
	kStrDeathMapDesc,
	kStrDeathMap,
	kStrBoots,
	kStrUseBoots,
	kStrWowCharmDesc,
	kStrWowCharm,
	kStrUseWowCharm,
	kStrWaterOpen,
	kStrDrinkIt,
	kStrItWorks,
	kStrSBOpen,
	kStrUsesFire,
	kStrMuscleDesc,
	kStrMuscle,
	kStrSBDesc,
	kStrSB,
	kStrFace,
	kStrFaceDesc,
	kStrTRNDesc,
	kStrTRN,
	kStrInvisDesc,
	kStrGoodLuckDesc,
	kStrAnaRing,
	kStrInvis,
	kStrGoesAway,
	kStrGiveHerRing,
	kStrGive2,
	kStrMadKingText,
	kStrMadKing3Text,
	kStrMadKing2Text,
	kStrDream1,
	kStrDream1P2,
	kStrDream1P3,
	kStrHowToGetOut,
	kStrSpore,
	kStrSporeDesc,
	kStrRequestPlayDisc,
	kStrOldGame,
	kStrEnterCertificate,
	kStrBadCertificate,
	kStrCert,
	kStrCert2,
	kStrTitle0,
	kStrTitle4,
	kStrMDesc,
	kStrM3Desc,
	kStrMapText1,
	kStrMapText2,

	// Level 0 str

	// Level 1 str

	// Level 2 str

	// Level 3 str

	// Level 4 str

	// Level 5 str

	// Level 6 str

	// Level 7 str
};

enum SObjType {
	kTypeTrap,
	kTypeCoin,
	kTypeWowCharm,
	kTypeDead,
	kTypeFireBall,
	kTypeDunRing,
	kTypeChest,
	kTypeDeathMap,
	kTypeWater,
	kTypeSpores,
	kTypeWormFood,
	kTypeChestKey,
	kTypePhant,
	kTypeGold,
	kTypeHay,
	kTypeBeam
};

enum SObjPickup {

};

enum SObjUse {

};

enum SDamage {

};

struct Damage {

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

 	SRoom(uint8 x, uint8 y, RoomFlag f) {
 			_x = x;
 			_y = y;
 		_flags = f;
 	}
};

struct SDoor {
DoorDir _dir = kLeft;
	uint8 _x = 0;
	uint8 _y = 0;
	uint8 _fromRoom = 0;
	uint8 _toRoom = 0;
	 bool _isLocked = false;

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
	 uint8 _x = 0;
	 uint8 _y = 0;
FPattern _pattern = kFlameOff;

 	SFlame(uint8 x, uint8 y, FPattern p) {
 			  _x = x;
 			  _y = y;
 	_pattern = p;
 	}
};

struct SObj {
	  uint8 _x = 0;
	  uint8 _y = 0;
   SObjType _type = kTypeTrap;
	  uint8 _flags = 0;
SpriteFrame _frame = kNoFrame;
Common::Array<uint8> _traps;

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











