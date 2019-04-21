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

#pragma once

typedef unsigned char byte;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;

struct icon_t {
	int16	sx;
	int16	sy;
	int16	ex;
	int16	ey;
	uint16	cursor_id;          // & 0x8000 - inactive/hidden
	unsigned int	action_id;
	unsigned int	object_id;
};
#define END_ICONS {-1, -1, -1, -1, 0, 0, 0}

struct room_t {
	byte   ff_0;
	byte   exits[4];
	byte   flags;
	uint16  bank;
	uint16  party;
	byte   level;
	byte   video;
	byte   location;
	byte   background;
};
#define END_ROOMS {0xFF, {0xFF, 0xFF, 0xFF, 0xFF}, 0xFF, 0xFFFF, 0xFFFF, 0xFF, 0xFF, 0xFF, 0xFF}

struct Follower {      // Characters on Mirror screen
	char        _id;         // character
	char        _spriteNum;      // sprite number
	int16       sx;
	int16       sy;
	int16       ex;
	int16       ey;
	int16       _spriteBank;
	int16       ff_C;
	int16       ff_E;
};

namespace PersonId {
enum PersonId {
	pidGregor = 0,                          // The King
	pidDina,                                // Pink dino
	pidTau,                                 // Late grandpa
	pidMonk,                                // Old wizard
	pidJabber,                              // Executioner
	pidEloi,                                // Evergreen ptero
	pidMungo,                               // Dina's husband
	pidEve,                                 // Blonde girl
	pidShazia,                              // Big boobs sis
	pidLeadersBegin,                        // 9
	pidChongOfChamaar = pidLeadersBegin,    // Dogface
	pidKommalaOfKoto,                       // Clones
	pidUlanOfUlele,                         // Shaman
	pidCabukaOfCantura,                     // Stone people
	pidMarindaOfEmbalmers,                  // Gods
	pidFuggOfTamara,                        // Boar-like
	pidThugg,                               // Bodyguard
	pidNarrator,                            // 16, Old Eloi, also BGM
	pidNarrim,                              // Sea snake
	pidMorkus,                              // Vicious tyran
	pidDinosaur,                            // different species of friendly dino
	pidEnemy                                // different species of enemy dino
};
}

namespace PersonMask {
enum PersonMask {
	pmGregor = 1,
	pmDina   = 2,
	pmTau    = 4,
	pmMonk   = 8,
	pmJabber = 0x10,
	pmEloi   = 0x20,
	pmMungo  = 0x40,
	pmEve    = 0x80,
	pmShazia = 0x100,
	pmLeader = 0x200,   // valley tribe leader
	pmThugg  = 0x400,
	pmQuest  = 0x800,   // special quest person
	pmDino   = 0x1000,
	pmEnemy  = 0x2000,
	pmMorkus = 0x4000
};
}

namespace PersonFlags {
enum PersonFlags {
	pfType0 = 0,
	pftTyrann,
	pfType2,
	pfType3,
	pfType4,
	pfType5,
	pfType6,
	pfType7,
	pfType8,
	pftMosasaurus,
	pftTriceraptor,
	pftVelociraptor,
	pfType12,
	pfType13,
	pfType14,
	pfType15,
	pfTypeMask = 0xF,
	pf10 = 0x10,
	pf20 = 0x20,
	pfInParty = 0x40,
	pf80 = 0x80
};
}

namespace Objects {
enum Objects {
	obNone,
	obWayStone,
	obShell,
	obTalisman,
	obTooth,
	obPrism,    // 5
	obFlute,
	obApple,
	obEgg,      // 8
	obRoot,
	obUnused10,
	obShroom,   // 11
	obBadShroom,    // 12
	obKnife,    // 13
	obNest, // 14
	obFullNest, // 15
	obGold,     // 16
	obMoonStone,
	obBag,
	obSunStone, // 19
	obHorn, // 20
	obSword,

	obMaskOfDeath,
	obMaskOfBonding,
	obMaskOfBirth,

	obEyeInTheStorm,    // 25
	obSkyHammer,
	obFireInTheClouds,
	obWithinAndWithout,
	obEyeInTheCyclone,
	obRiverThatWinds,

	obTrumpet,      // 31
	obUnused32,
	obDrum,
	obUnused34,
	obUnused35,
	obRing,

	obTablet1,      // 37 is 1st plaque, 6 total
	obTablet2,
	obTablet3,  // 39
	obTablet4,
	obTablet5,
	obTablet6
};
}

struct Goto {
	byte   _areaNum;    // target area
	byte   _curAreaNum; // current area
	byte   _enterVideoNum;
	byte   _travelTime;     // time to skip while in travel
	byte   _arriveVideoNum;
};

struct object_t {
	byte   _id;
	byte   _flags;
	int    _locations;      // index in kObjectLocations
	uint16  _itemMask;
	uint16  _powerMask;          // object of power bitmask
	int16  _count;
};

struct perso_t {
	uint16  _roomNum;    // room this person currently in
	uint16  _actionId;   // TODO: checkme
	uint16  _partyMask;      // party bit mask
	byte    _id;         // character
	byte    _flags;      // flags and kind
	byte    _roomBankId;// index in kPersoRoomBankTable for specific room banks
	byte    _spriteBank;       // sprite bank
	uint16  _items;      // inventory
	uint16  _powers;     // obj of power bitmask
	byte    _targetLoc;  // For party member this is mini sprite index
	byte    _lastLoc;    // For party member this is mini sprite x offset
	byte    _speed;      // num ticks per step
	byte    _steps;      // current ticks
};

struct Citadel {
	int16 _id;
	int16 _bank[8];
	int16 _video[8];
};

// A struct to hold the struct members of Common::Rect
struct Rect {
	int16   left, top, right, bottom;
};

namespace Areas {
enum Areas {
	arMo = 1,
	arTausCave,
	arChamaar,
	arUluru,
	arKoto,
	arTamara,
	arCantura,
	arShandovra,
	arNarimsCave,
	arEmbalmersCave,
	arWhiteArch,
	arMoorkusLair
};
}

namespace AreaFlags {
enum AreaFlags {
	afFlag1 = 1,
	afFlag2 = 2,
	afFlag4 = 4,
	afFlag8 = 8,
	afGaveGold = 0x10,
	afFlag20 = 0x20,

	HasTriceraptors = 0x100,
	HasVelociraptors = 0x200,
	HasTyrann = 0x400,

	TyrannSighted = 0x4000,
	afFlag8000 = 0x8000
};
}

struct Room {
	byte    _id;
	byte    _exits[4];   //TODO: signed?
	byte    _flags;
	uint16  _bank;
	uint16  _party;
	byte    _level;      // Citadel level
	byte    _video;
	byte    _location;
	byte    _backgroundBankNum; // bg/mirror image number (relative)
};

struct Area {
	byte   _num;
	byte   _type;
	uint16 _flags;
	uint16 _firstRoomIdx;
	byte   _citadelLevel;
	byte   _placeNum;
	Room  *_citadelRoomPtr;
	int16  _visitCount;
};

namespace AreaType {
enum AreaType {
	atCitadel = 1,
	atValley = 2,
	atCave = 3
};
}
