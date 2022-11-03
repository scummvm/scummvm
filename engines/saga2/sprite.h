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

#ifndef SAGA2_SPRITE_H
#define SAGA2_SPRITE_H

#include "saga2/rect.h"

namespace Saga2 {

class gPort;
class gPixelMap;

/* ===================================================================== *
   Basic sprite structures
 * ===================================================================== */

//  Sprite: A structure representing a single sprite

struct Sprite {
	Extent16        size;                   // size of sprite
	Point16         offset;                 // sprite origin point
	byte            *data;
	uint32			dataSize;

	Sprite(Common::SeekableReadStream *stream);
	~Sprite();

	// sprite data follows.
};

//  SpriteSet: A bunch of sprites in a single resource

struct SpriteSet {
	uint32           count;                  // number of images in the range
	Sprite           **sprites;
	// (variable-length array)
	// sprite structures follow table

	SpriteSet(Common::SeekableReadStream *stream);
	~SpriteSet();

	//  Member function to return a sprite from the set
	Sprite *sprite(int16 index) {
		return sprites[index];
	}

//  Sprite &operator[]( int32 index )
//  {
//      return (Sprite *)( (uint8 *)this + offsets[index] );
//  }

};

extern SpriteSet    *objectSprites,    // object sprites
       *mentalSprites,   // intagible object sprites
       *weaponSprites[], // weapon sprites
       *missileSprites;  // missile sprites

/* ===================================================================== *
   Describes the facing directions of actor sprites
 * ===================================================================== */

enum spriteFacingDirections {
	kSprFaceDown = 0,
	kSprFaceDownLeft,
	kSprFaceLeft,
	kSprFaceUpLeft,
	kSprFaceUp
};

/* ===================================================================== *
   ActorPose: Describes an element of a choreographed action
 * ===================================================================== */

const int           kNumPoseFacings = 8;

//  Describes a single entry in an actor sequence

struct ActorPose {

	//  Sequence element flags

	enum {
		//  Indicates which of the sprites should be drawn flipped
		//  left-to-right

		kActorFlipped        = (1 << 0),     // actor spr flipped left/right
		kLeftObjectFlipped   = (1 << 1),     // left hand object flipped
		kRightObjectFlipped  = (1 << 2),     // right hand object flipped

		//  Indicates the front-to-back priority of the objects

		kLeftObjectInFront   = (1 << 3),     // left object in front of actor
		kRightObjectInFront  = (1 << 4),     // right object in front of actor
		kLeftOverRight       = (1 << 5)      // left in front of right
	};

	uint16          flags;                  // sequence element flags

	uint8           actorFrameIndex,        // actor sprite index
	                actorFrameBank;         // which bank actor frame is in
	uint8           leftObjectIndex,        // index of obj in left hand
	                rightObjectIndex;       // index of obj in right hand

	Point16         leftObjectOffset,       // offset of left-hand obj.
	                rightObjectOffset;      // offset of right-hand obj.

	//  14 bytes

	ActorPose();
	ActorPose(Common::SeekableReadStream *stream);
	void load(Common::SeekableReadStream *stream);

	void write(Common::MemoryWriteStreamDynamic *out);
};

//  A choreographed sequence of frames

struct ActorAnimation {

	//  For each facing direction, the offset to the
	//  table of poses for that sequence, and the number of poses
	//  in the sequence.

	uint16 start[kNumPoseFacings];
	uint16 count[kNumPoseFacings];

	ActorAnimation(Common::SeekableReadStream *stream);

	// 32 bytes
};

struct ActorAnimSet {
	uint32 numAnimations;	// number of animations
	uint32 poseOffset;		// offset to poses table

	ActorAnimation **animations;
	ActorPose **poses;

	uint32 numPoses;
};

/* ===================================================================== *
   Sprite color lookup tables
 * ===================================================================== */

typedef uint8       ColorTable[256];

//  List of color schemes for sprites
struct ColorScheme {
	uint8           bank[11];
	uint8           speechColor;
	char            name[32];

	ColorScheme() {}
	ColorScheme(Common::SeekableReadStream *stream);
};

class ColorSchemeList {
public:
	int _count;
	ColorScheme **_schemes;

	ColorSchemeList(int count, Common::SeekableReadStream *stream);
	~ColorSchemeList();
};

/* ===================================================================== *
   Composite sprites (characters made from several component sprites)
 * ===================================================================== */

struct SpriteComponent {
	Sprite          *sp;                    // the sprite to draw
	Point16         offset;                 // offset from given origin
	uint8           *colorTable;            // color lookup table
	uint8           flipped;                // true if horizontally flipped.
};

enum spriteEffectFlags {
	kSprFXGhosted            = (1 << 0),     // semi-translucent dither
	kSprFXTerrainMask        = (1 << 1),     // mask sprite to terrain
	kSprFXGhostIfObscured    = (1 << 2)      // apply ghosted effect if
	// obscured by terrain
};


/* ===================================================================== *
   Object sprite information structure
 * ===================================================================== */

struct ObjectSpriteInfo {
	Sprite          *sp;                    // object sprite
	bool            flipped;                // mirror sprite horizontally
};

/* ===================================================================== *
   Actor Appearance
 * ===================================================================== */

//  Bits which represent the various "banks" of sprites for
//  each actor.

//  REM: I think we want more banks than this...

enum spriteBankNums {
	kSprStandBankNum = 0,
	kSprWalkBankNum,
	kSprRunBankNum,
	kSprKneelBankNum,
	kSprLeapBankNum,
	kSprClimbBankNum,
	kSprTalkBankNum,
	kSprFight1HBankNum,
	kSprFight2HBankNum,
	kSprFireBankNum,
	kSprPassiveBankNum,
	kSprUpStairsBankNum,
	kSprDnStairsBankNum,
	kSprSitBankNum,

	kSprBankCount
};

enum spriteBankBits {
	kSprStandBank    = (1 << kSprStandBankNum),
	kSprWalkBank     = (1 << kSprWalkBankNum),
	kSprRunBank      = (1 << kSprRunBankNum),
	kSprKneelBank    = (1 << kSprKneelBankNum),
	kSprLeapBank     = (1 << kSprLeapBankNum),
	kSprClimbBank    = (1 << kSprClimbBankNum),
	kSprTalkBank     = (1 << kSprTalkBankNum),
	kSprFight1HBank  = (1 << kSprFight1HBankNum),
	kSprFight2HBank  = (1 << kSprFight2HBankNum),
	kSprFireBank     = (1 << kSprFireBankNum),
	kSprPassiveBank  = (1 << kSprPassiveBankNum),
	kSprUpStairsBank = (1 << kSprUpStairsBankNum),
	kSprDnStairsBank = (1 << kSprDnStairsBankNum),
	kSprSitBank      = (1 << kSprSitBankNum)
};

//  This structure is used to contain all of the items needed
//  to draw an actor, including sprite set, frame list, and
//  wielding offsets.
//
//  There is an LRU cache of these structures maintained by
//  the sprite coordinator.

class ActorAppearance {
public:
	int16            _useCount;               // how many actors using this
	uint32           _id;

	ActorAnimSet    *_poseList;             // list of action sequences
	ColorSchemeList *_schemeList;           // color remapping info

	SpriteSet       *_spriteBanks[kSprBankCount];

	void loadSpriteBanks(int16 banksNeeded);

	//  Determine if this bank is loaded
	bool isBankLoaded(int16 bank) {
		return _spriteBanks[bank] != nullptr;
	}

	//  A request to load a bank.
	void requestBank(int16 bank) {
		//  Initiate a load of the sprite bank needed.
		if (!isBankLoaded(bank))
			loadSpriteBanks((int16)(1 << bank));
	}

	ActorAnimation *animation(int num) {
		if (_poseList == nullptr)
			return nullptr;

		if (num >= (int)_poseList->numAnimations) {
			warning("ActorPose:animation(), animation number is too high, %d >= %d", num, _poseList->numAnimations);
			return nullptr;
		}

		if (_poseList)
			return _poseList->animations[num];

		return nullptr;
	}

	ActorPose *pose(ActorAnimation *anim, int dir, int num) {
		if (_poseList == nullptr)
			return nullptr;

		if (num < 0 || num >= anim->count[dir])
			num = 0;

		num += anim->start[dir];

		if (num >= (int)_poseList->numPoses) {
			warning("ActorPose::pose(), pose number is too high, %d >= %d", num, _poseList->numPoses);
			return nullptr;
		}

		return _poseList->poses[num];
	}
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

void initSprites();
void cleanupSprites();

struct TilePoint;

//  Draw a plain sprite into a gPort, no masking or clipping
void DrawSprite(gPort &port, const Point16 &dest, Sprite *sp);

//  Draw a composite sprite with both masking and color mapping
void DrawCompositeMaskedSprite(
    gPort           &port,                  // destination gPort
    SpriteComponent *scList,                // list of components
    int16           numParts,               // number of components
    const Point16   &destPoint,             // where to render to
    const TilePoint &loc,                   // location on map
    int16           effects,                // effects flags
    bool            *obscured = NULL);      // set if object is obscured by terrain

//  Draw a single sprite with color mapping only
void DrawColorMappedSprite(
    gPort           &port,                  // destination gPort
    const Point16   &destPoint,             // where to render to
    Sprite          *sp,                    // sprite pointer
    uint8           *colorTable);           // color remapping table

//  Color map a sprite into a gPixelMap.
void ExpandColorMappedSprite(
    gPixelMap       &map,                   // destination gPixelMap
    Sprite          *sp,                    // sprite pointer
    uint8           *colorTable);           // color remapping table

//  Return a specific pixel from a sprite for mouse hit test
uint8 GetSpritePixel(
    Sprite          *sp,                    // sprite pointer
    int16           flipped,                // true if sprite was flipped
    const Point16   &testPoint);            // where to render to

//  Return the number of visible pixels in a sprite after terrain masking
uint16 visiblePixelsInSprite(
    Sprite          *sp,                    // sprite pointer
    bool            flipped,                // is sprite flipped
    ColorTable      colors,                 // sprite's color table
    Point16         drawPos,                // XY position of sprite
    TilePoint       loc,                    // UVZ coordinates of sprite
    uint16          roofID);                // ID of ripped roof

//  Assemble a color lookup table
void buildColorTable(
    uint8           *colorTable,            // color table to build
    uint8           *colorOptions,          // colors ranges chosen
    int16            numOptions);

//  Functions to load and release an actor appearance
ActorAppearance *LoadActorAppearance(uint32 id, int16 banksNeeded);
void ReleaseActorAppearance(ActorAppearance *aa);

} // end of namespace Saga2

#endif
