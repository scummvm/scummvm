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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_IDTYPES_H
#define SAGA2_IDTYPES_H

namespace Saga2 {

#define FTA

typedef uint32      ChunkID;
#define MakeID(a,b,c,d) ((d<<24L)|(c<<16L)|(b<<8L)|a)

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
                    WorldBaseID = 0xF000;   // 4K possible worlds

/* ===================================================================== *
   TileID
 * ===================================================================== */

typedef uint16      TileID;

const int16           nullID = -1;

/* ===================================================================== *
   PlayerActorID
 * ===================================================================== */

typedef int16   PlayerActorID;

/* ===================================================================== *
   MetaTileID struct
 * ===================================================================== */

struct StaticMetaTileID {
	int16 map, index;
};

struct MetaTileID {
	int16           map;            //  map number
	int16           index;          //  index into metatile array

	//  Default constructor
	MetaTileID() : map(0), index(0) {}

	//  Copy constructor
	MetaTileID(const MetaTileID &id) : map(id.map), index(id.index) {}

	//  Constructor
	MetaTileID(int16 m, int16 i) : map(m), index(i) {}

	MetaTileID(StaticMetaTileID mt) {
		map = mt.map;
		index = mt.index;
	}

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
extern const StaticMetaTileID NoMetaTile;

/* ===================================================================== *
   ActiveItemID struct
 * ===================================================================== */

const int   activeItemIndexMask = 0x1FFF;
const int   activeItemMapMask = 0xE000;
const int   activeItemMapShift = 13;

const int16 activeItemIndexNullID = 0x1FFF;

struct StaticActiveItemID {
	int16 val;
};

#include "common/pack-start.h"
struct ActiveItemID {
	int16           val;            //  ID value --
	//      first 3 bits world number
	//      next 13 bits index

	//  Default constructor
	ActiveItemID() : val(0) {}

	//  Copy constructor
	ActiveItemID(const ActiveItemID &id) : val(id.val) {
	}

	//  Constructor
	ActiveItemID(int16 idVal) : val(idVal) {}

	//  Constructor
	ActiveItemID(int16 m, int16 i) :
		val((m << activeItemMapShift) | (i & activeItemIndexMask)) {
	}

	ActiveItemID(StaticActiveItemID a) {
		val = a.val;
	}

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

	operator int16() {
		return val;
	}

	void setMapNum(int16 m) {
		val &= ~activeItemMapMask;
		val |= (m << activeItemMapShift);
	}

	int16 getMapNum() {
		return (uint16)val >> activeItemMapShift;
	}

	void setIndexNum(int16 i) {
		val &= ~activeItemIndexMask;
		val |= i & activeItemIndexMask;
	}

	int16 getIndexNum() {
		return val & activeItemIndexMask;
	}
} PACKED_STRUCT;
#include "common/pack-end.h"

//  ID of NULL active item
extern const StaticActiveItemID NoActiveItem;

/* ===================================================================== *
   Task's and TaskStacks
 * ===================================================================== */

//  Task evaluation return types
enum TaskResult {
	taskFailed      = -1,   //  Task has ended in failure
	taskNotDone     =  0,   //  Task has not ended yet
	taskSucceeded   =  1    //  Task has ended in success
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

typedef uint8 gPen;               // a pen index number

typedef uint16 weaponID;

typedef uint32 hResID;

typedef uint8       ColorTable[256];

#ifndef offsetof
#define offsetof(type,field) (uint32)&(((type *)0)->field)
#endif

#define maxuint8 0xff
#define maxint16 0x7fff
#define minint16 0x8000
#define maxuint16 0xffff
#define maxint32 0x7fffffff

enum {
	kActorListID = MKTAG('A', 'C', 'T', 'O')
};

// number of containers
const int   kNumViews    = 3;

enum {
	kNullWeapon = 0,
	kMaxWeapons = 256
};

enum {
	kActorCount = 575
};

//
// Damage effects - these are the types of damage in the world
//    Damage being defined as a change in effective vitality
//    Note that healing is negative damage.
//
enum effectDamageTypes {
	// Generic
	kDamageOther         = 0,    // Healing, cause wounds
	// Combat damage
	kDamageImpact        = 1,    // hammers, maces
	kDamageSlash         = 2,    // swords
	kDamageProjectile    = 3,            // arrows, poin-ted sticks
	// Magic damage
	kDamageFire          = 4,     // Yellow
	kDamageAcid          = 5,     // Violet
	kDamageHeat          = 6,     // Red
	kDamageCold          = 7,     // Blue
	kDamageLightning     = 8,     // Orange
	kDamagePoison        = 9,     // Green
	// Other magic damage
	kDamageMental        = 10,     // dain bramage
	kDamageToUndead      = 11,    // undead take this damage
	kDamageDirMagic      = 12,    // the plusses on swords etc.
	// Physiological Damage
	kDamageStarve        = 13,    // You must eat!
	// other
	kDamageEnergy        = 14     // Generally hard to resist - god damage
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

//	Size of a tile in ( U, V ) coords

enum {
	kTileUVSize = 16,
	kTileUVShift = 4,
	kTileZSize = 8,
	kTileZShift = 3,
	kTileUVMask = (kTileUVSize - 1)
};

//	Size of a map sector (4 metatiles x 4 metatiles)

enum {
	kSectorSize = kTileUVSize * 8 * 4,
	kSectorShift = kTileUVShift + 3 + 2,
	kSectorMask = (kSectorSize - 1)
};

//  Plaftorm metrics
enum {
	kPlatformWidth = 8,
	kPlatMask = kPlatformWidth - 1,
	kPlatShift = 3,
	kPlatUVSize = kTileUVSize * kPlatformWidth
};

//  Metatile metrics
enum {
    kMetaTileWidth = kTileWidth * kPlatformWidth,
    kMetaTileHeight = kTileHeight * kPlatformWidth,
    kMetaDX = kMetaTileWidth / 2,
    kMetaDY = kMetaTileHeight / 2
};

enum {
    kSubTileSize = 4,
    kSubTileMask = kSubTileSize - 1,
    kSubTileShift = 2,
    kTileSubSize = 4,
    kTileSubMask = kTileSubSize - 1,
    kTileSubShift = 2
};

//  Constants to convert an X,Y into subtile coordinates
enum {
    kSubTileDX = (kTileDX / 4),
    kSubTileDY = (kTileDY / 4),
    kSubTileDXShift = (kTileDXShift - 2),
    kSubTileDYShift = (kTileDYShift - 2)
};

enum {
    kSubTileMaskUShift = 4,
    kSubTileMaskVShift = 1
};

//  Maximum height that a character can climb w/o steps or ladders
enum {
    kMaxStepHeight   = 16,   // highest climbable step
    kMaxPickHeight   = 64,   // highest pickable step
    kMaxSmoothStep   = 8,    // highest smoothly climbable
    kMaxJumpStep     = 64    // highest jump character likes
};


// Save/Load dialog metrics

enum {
	kNumSaveLoadPanels   = 3,
	kNumSaveLoadBtns     = 4,
	kNumSaveLoadTexts    = 1,

	kSLDBoxXSize = 374,
	kSLDBoxXSzNS = 366,
	kSLDBoxYSize = 223,
	kSLDBoxX     = (640 - kSLDBoxXSize) / 2,
	kSLDBoxY     = (480 - kSLDBoxYSize) / 3,

	kSLTPHeight = 38,
	kSLMDHeight = 122,
	kSLBTHeight = 63,
	kSLTPWidth  = 374,
	kSLMDWidth  = 374,
	kSLBTWidth  = 374
};

// Options dialog metrics
enum {
	kNumOptionsPanels    = 3,
	kNumOptionsBtns      = 9,
	kNumOptionsTexts     = 8,

	kOptBoxXSize = 487,
	kOptBoxXSzNS = 479,
	kOptBoxYSize = 230,
	kOptBoxX     = (640 - kOptBoxXSize) / 2,
	kOptBoxY     = (480 - kOptBoxYSize) / 3,

	kOptTPHeight = 39,
	kOptMDHeight = 90,
	kOptBTHeight = 101,
	kOptTPWidth  = 487,
	kOptMDWidth  = 487,
	kOptBTWidth  = 487
};

// buttons
enum {
	kButtonSpace         =   3,
	kButtonYOffset       =   kOptTPHeight + 7,
	kPushButtonWidth     =   121,
	kPushButtonHeight    =   30,

	kSliderWidth         =   168,
	kImageHeight         =   17,

	kTextPixelLen        =   175,
	kSmallTextOffset     =    80
};

// Message Dialog Metrics
enum {
	kNumMessagePanels    = 1,
	kNumMessageBtns      = 3,
	kNumMessageTexts     = 2,
	kMesBtnOffset        = 14,

	kMesBoxXSize = 374,
	kMesBoxXSzNS = 368,
	kMesBoxYSize = 146,
	kMesBoxX     = (640 - kMesBoxXSize) / 2,
	kMesBoxY     = (480 - kMesBoxYSize) / 3
};

//Sets Up Tile Map Area
enum {
	kTileRectX      = 16 + 4,
	kTileRectY      = 16 + 4,
	kTileRectWidth  = 448 - 8,
	kTileRectHeight = 428 - 8
};

//  Horribly kludged hard-coded sprite index numbers for bubble sprites
enum {
	kMaxActiveSpells = 8,
	kBaseBubbleSpriteIndex = 111,
	kBubbleSpriteCount = 8
};

enum {
	kPlayerActors = 3,
	kMinAutoAggressionVitality = 5,
	BASE_REC_RATE = 1
};

enum {
	kObjectVolumeArraySize = 128
};

enum {
	kDefaultReach = 24
};

// Actor Constants
enum {
	kMaxFactions = 64
};

} // end of namespace Saga2

#endif
