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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_IDTYPES_H
#define SAGA2_IDTYPES_H

namespace Saga2 {

/* ===================================================================== *
   ObjectID
 * ===================================================================== */

typedef uint16      ObjectID;               // a reference to a script

//  Some fixed objects

const ObjectID      Nothing = 0,            // a reference to no object
                    ObjectLimbo = 1,        // where dead objects go
                    ActorLimbo = 2,         // where dead actors go
                    ImportantLimbo = 3,     // where dead important objects go
                    ActorBaseID = 0x8000,   // high bit set for actors
                    WorldBaseID = 0xF000;   // 4K posible worlds

/* ===================================================================== *
   TileID
 * ===================================================================== */

typedef uint16      TileID;

const int           nullID = 0xffff;

/* ===================================================================== *
   PlayerActorID
 * ===================================================================== */

typedef int16   PlayerActorID;

/* ===================================================================== *
   MetaTileID struct
 * ===================================================================== */

struct MetaTileID {
	int16           map;            //  map number
	int16           index;          //  index into metatile array

	//  Default constructor
	MetaTileID(void) {}

	//  Copy constructor
	MetaTileID(const MetaTileID &id) : map(id.map), index(id.index) {}

	//  Constructor
	MetaTileID(int16 m, int16 i) : map(m), index(i) {}

	MetaTileID operator = (const MetaTileID &id) {
		map = id.map;
		index = id.index;
		return *this;
	}

	bool operator == (const MetaTileID &id) const {
		return map == id.map && index == id.index;
	}

	bool operator != (const MetaTileID &id) const {
		return map != id.map || index != id.index;
	}
};

//  ID of NULL meta tile
extern const MetaTileID     NoMetaTile;

/* ===================================================================== *
   ActiveItemID struct
 * ===================================================================== */

const int   activeItemIndexMask = 0x1FFF;
const int   activeItemMapMask = 0xE000;
const int   activeItemMapShift = 13;

const int16 activeItemIndexNullID = 0x1FFF;

class ActiveItemID {
private:
	int16           val;            //  ID value --
	//      first 3 bits world number
	//      next 13 bits index

public:
	//  Default constructor
	ActiveItemID(void) {}

	//  Copy constructor
	ActiveItemID(const ActiveItemID &id) : val(id.val) {
	}

	//  Constructor
	ActiveItemID(int16 idVal) : val(idVal) {}

	//  Constructor
#if DEBUG
	ActiveItemID(int16 m, int16 i);
#else
	ActiveItemID(int16 m, int16 i) :
		val((m << activeItemMapShift) | (i & activeItemIndexMask)) {
	}
#endif

	ActiveItemID operator = (const ActiveItemID &id) {
		val = id.val;
		return *this;
	}

	ActiveItemID operator = (int16 idVal) {
		val = idVal;
		return *this;
	}

	bool operator == (const ActiveItemID &id) const {
		return val == id.val;
	}

	bool operator != (const ActiveItemID &id) const {
		return val != id.val;
	}

	operator int16(void) {
		return val;
	}

#if DEBUG
	void setMapNum(int16 m);
#else
	void setMapNum(int16 m) {
		val &= ~activeItemMapMask;
		val |= (m << activeItemMapShift);
	}
#endif

	int16 getMapNum(void) {
		return (uint16)val >> activeItemMapShift;
	}

#if DEBUG
	void setIndexNum(int16 i);
#else
	void setIndexNum(int16 i) {
		val &= ~activeItemIndexMask;
		val |= i & activeItemIndexMask;
	}
#endif

	int16 getIndexNum(void) {
		return val & activeItemIndexMask;
	}
};

//  ID of NULL active item
extern const ActiveItemID   NoActiveItem;

/* ===================================================================== *
   Task's and TaskStacks
 * ===================================================================== */

//  Task evaluation return types
enum TaskResult {
	taskFailed      = -1,   //  Task has ended in failure
	taskNotDone     =  0,   //  Task has not ended yet
	taskSucceeded   =  1,   //  Task has ended in success
};

typedef int16   TaskID;
const TaskID    NoTask = -1;

typedef int16       TaskStackID;
const TaskStackID   NoTaskStack = -1;

/* ===================================================================== *
   TimerID
 * ===================================================================== */

typedef int16 TimerID;

/* ===================================================================== *
   SensorID
 * ===================================================================== */

typedef int16 SensorID;

/* ===================================================================== *
   BandID
 * ===================================================================== */

typedef int16   BandID;
const BandID    NoBand = -1;

typedef uint32 soundSegment;

typedef uint8 gPen;               // a pen index number

typedef uint16 weaponID;

typedef uint32 hResID;

typedef uint8       ColorTable[256];

// number of containers
const int   kNumViews    = 3;

enum {
	kMaxWeapons = 256
};

//
// Damage effects - these are the types of damage in the world
//    Damage being defined as a change in effective vitality
//    Note that healing is negative damage.
//
enum effectDamageTypes {
	// Generic
	damageOther         = 0,    // Healing, cause wounds
	// Combat damage
	damageImpact        = 1,    // hammers, maces
	damageSlash         = 2,    // swords
	damageProjectile    = 3,            // arrows, poin-ted sticks
	// Magic damage
	damageFire          = 4,     // Yellow
	damageAcid          = 5,     // Violet
	damageHeat          = 6,     // Red
	damageCold          = 7,     // Blue
	damageLightning     = 8,     // Orange
	damagePoison        = 9,     // Green
	// Other magic damage
	damageMental        = 10,     // dain bramage
	damageToUndead      = 11,    // undead take this damage
	damageDirMagic      = 12,    // the plusses on swords etc.
	// Physiological Damage
	damageStarve        = 13,    // You must eat!
	// other
	damageEnergy        = 14     // Generally hard to resist - god damage
};

//  Tile metrics

enum {
	kTileWidth = 64,
	kTileHeight = 32,
	kTileMaxHeight = 160,
	kTileDX = (kTileWidth / 2),
	kTileDY = (kTileHeight / 2),
	kTileDXShift = 5,
	kTileDYShift = 4,
	kTileDXMask = (kTileDX - 1),
	kTileDYMask = (kTileDY - 1),
	kMaxTileHeight = 160
};

} // end of namespace Saga2

#endif
