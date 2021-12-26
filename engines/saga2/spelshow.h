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

#ifndef SAGA2_SPELSHOW_H
#define SAGA2_SPELSHOW_H

#include "saga2/dispnode.h"
#include "saga2/speldefs.h"

namespace Saga2 {

//-----------------------------------------------------------------------
//	Effectron List

typedef DisplayNodeList EffectronList;

//-----------------------------------------------------------------------
// Spell control functions
//   Various instances of these can be used in different combinations to
//   yield different shaped spells. Each spell will need one each of
//   these types of functions:
//
//   Effectron Status
//     This returns the state of a given effectron as a function of time
//
//   Spell Spritation
//     Returns a sprite facing as a function of time.
//
//   Spell Location
//     Returns the position of the effectron as a function of time
//
//   Spell Height
//     This is NOT the effectron's Z coordinate. It is the height used
//     for collision detection purposes
//
//   Spell Breadth
//     This is the effectron's cross section for collision detection
//     purposes.
//

//-----------------------------------------------------------------------
//	Spell Status

typedef EffectronFlags SpellStatusFunction(Effectron *);

#define SPELLSTATUSFUNCTION(fname) EffectronFlags fname( Effectron * effectron )

//-----------------------------------------------------------------------
//	Spell Spritation

typedef SpellPoseID SpellSpritationFunction(const Effectron *const);

#define SPELLSPRITATIONFUNCTION(fname) SpellPoseID fname( const Effectron * const effectron )

//-----------------------------------------------------------------------
//	Spell Location Calls

typedef TilePoint SpellLocationFunction(const Effectron *const);

#define SPELLLOCATIONFUNCTION(fname) TilePoint fname( const Effectron * const effectron )

//-----------------------------------------------------------------------
//	Spell Height

typedef spellHeight SpellHeightFunction(const Effectron *const);

#define SPELLHEIGHTFUNCTION(fname) spellHeight fname( const Effectron * const effectron )

//-----------------------------------------------------------------------
//	Spell width

typedef spellBreadth SpellBreadthFunction(const Effectron *const);

#define SPELLBREADTHFUNCTION(fname) spellBreadth fname( const Effectron * const effectron )

//-----------------------------------------------------------------------
//	Spell init

typedef void SpellInitFunction(Effectron *);

#define SPELLINITFUNCTION(fname) void fname( Effectron * effectron )


//-----------------------------------------------------------------------
// EffectronDisplayPrototype
//   This tracks the functions needed to display a particular type of
//   spell on the screen. (Ball spells, bolt spells, etc)
//

class EffectDisplayPrototype {
	static SPELLLOCATIONFUNCTION(nullLocation) {
		return TilePoint(0, 0, 0);
	}
	static SPELLSPRITATIONFUNCTION(nullSpritation) {
		return 0;
	}
	static SPELLSTATUSFUNCTION(nullStatus) {
		return effectronDead;
	}
	static SPELLHEIGHTFUNCTION(nullHeight) {
		return 0;
	}
	static SPELLBREADTHFUNCTION(nullBreadth) {
		return 0;
	}
	static SPELLINITFUNCTION(nullInit) {
	}

	EffectID                ID;
public:
	int16                   nodeCount;
	EffectDisplayPrototype  *next;

	SpellLocationFunction   *location;
	SpellSpritationFunction *spriteno;
	SpellStatusFunction     *status;
	SpellHeightFunction     *height;
	SpellBreadthFunction    *breadth;
	SpellInitFunction       *init;

	EffectDisplayPrototype() {
		nodeCount = 0;
		next = NULL;
		location = &nullLocation;
		spriteno = &nullSpritation;
		status  = &nullStatus;
		height  = &nullHeight;
		breadth = &nullBreadth;
		init    = &nullInit;
	}

	EffectDisplayPrototype(
	    int16                   nodes,
	    SpellLocationFunction   *newLocation,
	    SpellSpritationFunction *newSpriteno,
	    SpellStatusFunction     *newStatus,
	    SpellHeightFunction     *newHeight,
	    SpellBreadthFunction    *newBreadth,
	    SpellInitFunction       *newInit);
	~EffectDisplayPrototype() {
		if (next) delete next;
		next = NULL;
	}
	void setID(EffectID i) {
		ID = i;
	}
	EffectID thisID() {
		return ID;
	}
};

typedef EffectDisplayPrototype *pEffectDisplayPrototype;

//-----------------------------------------------------------------------
//	Effect Display Prototype List
//
// This class embodies a global list of EDPs
//


class EffectDisplayPrototypeList {
	pEffectDisplayPrototype         *effects;
	uint16                          count;
	uint16                          maxCount;

public:
	EffectDisplayPrototypeList(int32 c);
	~EffectDisplayPrototypeList();

	int32 add(EffectDisplayPrototype *edp) ;
	void cleanup();
	void append(EffectDisplayPrototype *edp, int32 acount);
	EffectDisplayPrototype *operator[](EffectID e);
};

//-----------------------------------------------------------------------
//	Effectron collision flags
//
//  need to track whether things
//    bounce/die/stop/ignore
//  when hitting
//    actors/objects/terrain
//

enum effectCollisionCont {
	ecFlagNone = 0,
	ecFlagBounce,
	ecFlagDie,
	ecFlagStop
};

enum effectDirectionInit {
	diFlagZero = 0,
	diFlagInc = 1,
	diFlagInc2 = 2,
	diFlagInc3 = 3,
	diFlagInc4 = 4,
	diFlagRand = 5
};


//-----------------------------------------------------------------------
//  SpellDisplayPrototype
//	  All the information needed to display a spell
//    Combines a SpellEffectPrototype with the appropriate sprites

class SpellDisplayPrototype {
	SpellID             ID;
public:
	EffectID            effect;        // Effect ID
	int32               effParm1;      //   effect setting 1
	int32               effParm2;      //   effect setting 1
	int32               effParm3;      //   effect setting 1
	int32               effParm4;      //   effect setting 1

	effectDirectionInit scatter;       // direction init mode
	effectCollisionCont elasticity;    // collision flags

	SpellAge            maxAge;        // auto self-destruct age
	SpellAge            implementAge;        // auto self-destruct age
	uint32              primarySpriteID;   // RES_ID(x, y, z, 0) to get sprites
	uint8               primarySpriteNo;   // sprites available
	uint32              secondarySpriteID;   // RES_ID(x, y, z, 0) to get sprites
	uint8               secondarySpriteNo;   // sprites available
	//uint8             effCount;      // effectrons to allocate

	uint8               colorMap[4];          // indirect color map
	// full init
	SpellDisplayPrototype(
	    EffectID, int32, int32, int32, int32, effectDirectionInit,
	    effectCollisionCont, SpellAge, uint32, uint8, uint8);

	SpellDisplayPrototype(ResourceSpellItem *rsi);

	void getColorTranslation(ColorTable mainColors, Effectron *);        // colors for effectrons
	void setID(SpellID i) {
		ID = i;
	}
	SpellID thisID() {
		return ID;
	}
};

//-----------------------------------------------------------------------
//	SpellDisplayPrototypeList
//    A global list of sdp's

typedef SpellDisplayPrototype *pSpellDisplayPrototype;

class SpellDisplayPrototypeList {
	pSpellDisplayPrototype              *spells;
	uint16                              count;
	uint16                              maxCount;

public:
	SpellDisplayPrototypeList(uint16 s);
	~SpellDisplayPrototypeList();

	void init();
	void cleanup();
	int32 add(SpellDisplayPrototype *sdp);
	SpellDisplayPrototype *operator[](SpellID s);
};

//-----------------------------------------------------------------------
//	SpellInstance
//    When a SpellDisplayPrototype is instantiated, one of these is created
//    and the main display loop updates it till it dies

class SpellInstance {
	friend struct StorageSpellInstance;

	SpellAge                implementAge;   // age at which to implement the spell effects
public:
	EffectDisplayPrototype  *effect;        // effect prototype of the current effect
	SpellDisplayPrototype   *dProto;        // effect prototype of the current effect
	SpellCaster            *caster;
	SpellTarget            *target;
	GameWorld              *world;
	SpellAge                age;
	EffectronList           eList;
	SpellID                 spell;
	SpellAge                maxAge;
	int16                   effSeq;         // which effect in a sequence is being played

	SpellInstance(SpellCaster *newCaster, SpellTarget *newTarget, SpellID);
	SpellInstance(SpellCaster *newCaster, GameObject  &newTarget, SpellID);
	SpellInstance(SpellCaster *newCaster, GameObject  *newTarget, SpellID);
	SpellInstance(SpellCaster *newCaster, TilePoint   &newTarget, SpellID);
	SpellInstance(StorageSpellInstance &ssi);
	~SpellInstance();

	void init();
	void initEffect(TilePoint);
	void readEffect(Common::InSaveFile *in, uint16 eListSize);
	void writeEffect(Common::MemoryWriteStreamDynamic *out);
	void termEffect();
	size_t saveSize();

	bool buildList();
	bool updateStates(int32 deltaTime);
};

//-----------------------------------------------------------------------
//	SpellDisplayList
//    This class is used to keep track of all the spells currently
//    displaying effectrons

typedef SpellInstance *pSpellInstance;

class SpellDisplayList {
	uint16                      count;
	uint16                      maxCount;
public :
	pSpellInstance              *spells;

	void init();
	void cleanup();
	SpellDisplayList(uint16 s);
	~SpellDisplayList();

	void add(SpellInstance *newSpell);

	void tidyKill(uint16 spellNo);

	void buildList();
	void updateStates(int32 deltaTime);

	void write(Common::OutSaveFile *outD);
	void read(Common::InSaveFile *in);
	void wipe();
	size_t saveSize();
};


/* ===================================================================== *
   Inlines
 * ===================================================================== */

//-----------------------------------------------------------------------
// Some functions that require the above definitions to work

inline GameWorld *Effectron::world() const {
	return parent->world;
}
inline int16 Effectron::getMapNum() const {
	return parent->world->mapNum;
}

inline EffectID Effectron::spellID() {
	return parent->spell;
}
inline SpellDisplayPrototype *Effectron::spell() {
	return (*g_vm->_sdpList)[(SpellID) spellID()];
}
inline EffectID Effectron::effectID() {
	return spell()->effect;
}
inline EffectDisplayPrototype *Effectron::effect() {
	return parent->effect;
}
inline EffectronFlags Effectron::staCall() {
	return parent->effect->status(this);
}
inline TilePoint Effectron::posCall() {
	return parent->effect->location(this);
}
inline SpellSpritationSeed Effectron::sprCall() {
	return parent->effect->spriteno(this);
}
inline spellHeight Effectron::hgtCall() {
	return parent->effect->height(this);
}
inline spellBreadth Effectron::brdCall() {
	return parent->effect->breadth(this);
}
inline void Effectron::initCall(int16 eno) {
	partno = eno;
	parent->effect->init(this);
}

/* ===================================================================== *
   prototypes
 * ===================================================================== */

int16 whichColorMap(EffectID eid, const Effectron *const effectron);

//-----------------------------------------------------------------------
// Spell building block functions
//   These are the functions available to update various aspects
//   of Effectrons

SPELLSTATUSFUNCTION(invisibleSpellSta);
SPELLSTATUSFUNCTION(auraSpellSta);
SPELLSTATUSFUNCTION(projectileSpellSta);
SPELLSTATUSFUNCTION(exchangeSpellSta);
SPELLSTATUSFUNCTION(boltSpellSta);
SPELLSTATUSFUNCTION(coneSpellSta);
SPELLSTATUSFUNCTION(ballSpellSta);
SPELLSTATUSFUNCTION(squareSpellSta);
SPELLSTATUSFUNCTION(waveSpellSta);
SPELLSTATUSFUNCTION(stormSpellSta);
SPELLSTATUSFUNCTION(beamSpellSta);
SPELLSTATUSFUNCTION(wallSpellSta);

SPELLLOCATIONFUNCTION(invisibleSpellPos);
SPELLLOCATIONFUNCTION(auraSpellPos);
SPELLLOCATIONFUNCTION(projectileSpellPos);
SPELLLOCATIONFUNCTION(exchangeSpellPos);
SPELLLOCATIONFUNCTION(boltSpellPos);
SPELLLOCATIONFUNCTION(coneSpellPos);
SPELLLOCATIONFUNCTION(ballSpellPos);
SPELLLOCATIONFUNCTION(squareSpellPos);
SPELLLOCATIONFUNCTION(waveSpellPos);
SPELLLOCATIONFUNCTION(stormSpellPos);
SPELLLOCATIONFUNCTION(beamSpellPos);
SPELLLOCATIONFUNCTION(wallSpellPos);
SPELLLOCATIONFUNCTION(glowSpellPos);

SPELLSPRITATIONFUNCTION(invisibleSprites);
SPELLSPRITATIONFUNCTION(auraSprites);
SPELLSPRITATIONFUNCTION(projectileSprites);
SPELLSPRITATIONFUNCTION(exchangeSprites);
SPELLSPRITATIONFUNCTION(boltSprites);
SPELLSPRITATIONFUNCTION(coneSprites);
SPELLSPRITATIONFUNCTION(ballSprites);
SPELLSPRITATIONFUNCTION(squareSprites);
SPELLSPRITATIONFUNCTION(waveSprites);
SPELLSPRITATIONFUNCTION(stormSprites);
SPELLSPRITATIONFUNCTION(beamSprites);
SPELLSPRITATIONFUNCTION(wallSprites);

SPELLHEIGHTFUNCTION(ShortTillThere);
SPELLHEIGHTFUNCTION(GrowLinear);
SPELLBREADTHFUNCTION(StaticHeight);

SPELLBREADTHFUNCTION(ThinTillThere);
SPELLBREADTHFUNCTION(BulkLinear);
SPELLBREADTHFUNCTION(StaticBreadth);

SPELLINITFUNCTION(invisibleSpellInit);
SPELLINITFUNCTION(auraSpellInit);
SPELLINITFUNCTION(projectileSpellInit);
SPELLINITFUNCTION(exchangeSpellInit);
SPELLINITFUNCTION(boltSpellInit);
SPELLINITFUNCTION(coneSpellInit);
SPELLINITFUNCTION(ballSpellInit);
SPELLINITFUNCTION(squareSpellInit);
SPELLINITFUNCTION(waveSpellInit);
SPELLINITFUNCTION(stormSpellInit);
SPELLINITFUNCTION(glowSpellInit);
SPELLINITFUNCTION(beamSpellInit);
SPELLINITFUNCTION(wallSpellInit);

} // end of namespace Saga2

#endif  //SPELLBUK_H
