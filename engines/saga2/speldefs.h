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

#ifndef SAGA2_SPELDEFS_H
#define SAGA2_SPELDEFS_H

#include "saga2/tile.h"
#include "saga2/objects.h"

namespace Saga2 {

struct StorageSpellTarget;
class SpellInstance;
struct StorageEffectron;
class SpellDisplayPrototype;
class EffectDisplayPrototype;
struct ResourceSpellItem;
struct StorageSpellInstance;

/* ===================================================================== *
   Constants
 * ===================================================================== */

// this determines the maimum nuber of spells the system can handle.

#define MAX_SPELLS 128
#define continuouslyImplemented (-1)

/* ===================================================================== *
   Types
 * ===================================================================== */

//-----------------------------------------------------------------------
//	index into the Visual Effect list

typedef uint16 EffectID;

//-----------------------------------------------------------------------
//	Spell Sprite Pose

typedef uint16 SpellPoseID;

//-----------------------------------------------------------------------
//	height to use for collision detection

typedef uint8 spellHeight;

//-----------------------------------------------------------------------
//	breadth to use for collision detection

typedef uint8 spellBreadth;

//-----------------------------------------------------------------------
//	SpellAge type

typedef int32 SpellAge;

//-----------------------------------------------------------------------
//	SpellPositionSeed type
//    Both a location indicator and a direction indicator can be
//    gotten from this type

typedef int32 SpellPositionSeed;

// Get the Position part of the seed
inline uint16 SpellPosition(SpellPositionSeed sps) {
	return sps & 0xFFFF;
}

// Get the Direction part of the seed (used in explosions)
inline uint16 SpellDirection(SpellPositionSeed sps) {
	return ((sps >> 16) & 0xFFFF);
}

//-----------------------------------------------------------------------
//	SpellSpritationSeed type

typedef int32 SpellSpritationSeed;

//-----------------------------------------------------------------------
//	SpellCaster type (identical to GameObject)

typedef GameObject SpellCaster;

/* ===================================================================== *
   Misplaced
 * ===================================================================== */

// some cheesy inlines for extracting location info
//   they should probably be member functions of other classes but
//   I didn't want to modify those headers

inline TilePoint TAGPos(ActiveItem *ai) {
	if (ai == NULL) return Nowhere;
	assert(ai->_data.itemType == activeTypeInstance);
	return TilePoint(
	           ai->_data.instance.u << kTileUVShift,
	           ai->_data.instance.v << kTileUVShift,
	           ai->_data.instance.h);
}

inline TilePoint objPos(GameObject *go) {
	if (go == NULL) return Nowhere;
	TilePoint t = go->getWorldLocation();
	t.z += go->proto()->height / 2;
	return t;
}

/* ===================================================================== *
   another type
 * ===================================================================== */

//-----------------------------------------------------------------------
//	SpellTarget type
//    allows access to several types of targets with the same code

class SpellTarget {
	friend struct StorageSpellTarget;

public :
	enum spellTargetType {
		spellTargetNone         = 0,        // invalid
		spellTargetPoint,                   // targeted on a particular point
		spellTargetObjectPoint,             // targeted on an object's location
		spellTargetObject,                  // targeted on an object (tracking)
		spellTargetTAG                      // targeted on an object (tracking)
	};

private:
	spellTargetType type;

	TilePoint       loc;
	GameObject      *obj;
	ActiveItem      *tag;

public:
	SpellTarget     *next;

	SpellTarget() {
		type = spellTargetNone;
		obj = NULL;
		loc.u = 0;
		loc.v = 0;
		loc.z = 0;
		next = NULL;
	}

	// This constructor is for non-tracking targets
	SpellTarget(GameObject &object) {
		type = spellTargetObjectPoint;
		loc = object.getWorldLocation();
		loc.z += object.proto()->height / 2;
		next = NULL;
		obj = &object;
	}

	// This constructor is for tracking targets
	SpellTarget(GameObject *object) {
		type = spellTargetObject;
		obj = object;
		next = NULL;
	}
	SpellTarget(TilePoint &tp) {
		type = spellTargetPoint;
		loc = tp;
		next = NULL;
	}
	SpellTarget(ActiveItem *ai) {
		type = spellTargetTAG;
		tag = ai;
		next = NULL;
	}
	SpellTarget(StorageSpellTarget &sst);

	SpellTarget &operator=(const SpellTarget &src) {
		type = src.type;
		loc = src.loc;
		obj = src.obj;
		tag = src.tag;
		next = src.next;
		return *this;
	}
	SpellTarget(const SpellTarget &src) {
		type = src.type;
		loc = src.loc;
		obj = src.obj;
		tag = src.tag;
		next = src.next;
	}

	~SpellTarget() {
		if (next) delete next;
		next = NULL;
	};

	TilePoint getPoint(void) {
		switch (type) {
		case spellTargetPoint       :
		case spellTargetObjectPoint :
			return loc;
		case spellTargetObject      :
			return objPos(obj);
		case spellTargetTAG         :
			return TAGPos(tag);
		case spellTargetNone        :
		default                     :
			return Nowhere;
		}
	}

	spellTargetType getType(void) {
		return type;
	}

	GameObject *getObject(void) {
		assert(type == spellTargetObject);
		return obj;
	}

	ActiveItem *getTAG(void) {
		assert(type == spellTargetTAG);
		return tag;
	}

};

//-----------------------------------------------------------------------
//	Effectron flags

enum EffectronFlagMasks {
	effectronOK     = 0,
	effectronHidden = (1 << 0),
	effectronDead   = (1 << 1),
	effectronBumped = (1 << 2)
};

typedef uint32 EffectronFlags;

//-----------------------------------------------------------------------
//	EffectronSize Type

typedef Extent16 EffectronSize;

//-----------------------------------------------------------------------
//	Effectron

class Effectron {
	friend struct StorageEffectron;

	EffectronFlags          flags;            // this effectrons status
	EffectronSize           size;             // this effectrons size
	Rect16                  hitBox;           // hitbox for clicking this item
	// dispnode needs the latter

public:
	SpellInstance           *parent;          // pointer back to the spell that spawned this

	int16                   partno;           // Which effectron in a group this represents
	Point16                 screenCoords;     // screen coordinates last drawn at

	TilePoint               start,            // travelling from
	                        finish,           // travelling to
	                        current,          // current position
	                        velocity,         // current velocity
	                        acceleration;     // current acceleration
	uint16                  totalSteps,       // discrete jumps in the path
	                        stepNo;           // current jump

	spellHeight             hgt;              // collision detection stuff
	spellBreadth            brd;

	SpellPositionSeed       pos;              // These three are part of an old way of
	SpellSpritationSeed     spr;              // updating effectrons
	SpellAge                age;



	Effectron();
	Effectron(uint16 newPos, uint16 newDir);
	Effectron(StorageEffectron &se, SpellInstance *si);

	void drawEffect(void);
	void updateEffect(int32 deltaTime);

	inline TilePoint SpellPos(void) {
		return current;
	}
	inline int32 spriteID(void)     {
		return spr;
	}

	inline void hide(void)     {
		flags |= effectronHidden;
	}
	inline void unhide(void)   {
		flags &= (~effectronHidden);
	}
	inline bool isHidden(void) const {
		return flags & effectronHidden;
	}
	inline void kill(void)     {
		flags |= effectronDead;
	}
	inline int isDead(void) const      {
		return flags & effectronDead;
	}
	inline void bump(void);
	inline int isBumped(void) const        {
		return flags & effectronBumped;
	}

	inline GameWorld *world(void) const;
	inline int16 getMapNum(void) const;

	inline EffectID                 spellID(void);
	inline SpellDisplayPrototype    *spell(void);
	inline EffectID                 effectID(void);
	inline EffectDisplayPrototype   *effect(void);
	inline EffectronFlags       staCall(void);
	inline TilePoint            posCall(void);
	inline SpellSpritationSeed  sprCall(void);
	inline spellHeight          hgtCall(void);
	inline spellBreadth         brdCall(void);
	inline void                 initCall(int16);
};

} // end of namespace Saga2

#endif
