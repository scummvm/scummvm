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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/objproto.h"
#include "saga2/spellbuk.h"
#include "saga2/spelshow.h"
#include "saga2/spellio.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

//-------------------------------------------------------------------
// The initialization for spells is done in this module:
//  - Display Effects are defined (ball spell, bolt spell, etc)
//  - The spell definitions are loaded from the resource file
//  - The spell internal effect definitions are loaded from disk
//    and attached to the appropriate spell
//  - Spell Color maps are loaded from the resource file
//  - This file is also home to most of the global variables
//    which relate to spell casting
//

/* ===================================================================== *
   Constants
 * ===================================================================== */

const uint32                    spellSpriteID = MKTAG('S', 'P', 'F', 'X');

const int32 maxSpells = totalSpellBookPages;
const int32 maxSpellPrototypes = totalSpellBookPages;
const int32 maxEffectPrototypes = 16;

const int32 maxSpellColorMaps = 32;

/* const */     // For some reason, MVC can't handle constant static classes.
// ( Note: It would be better to use a SIN/COS table anyway than
// a table of points, since that allows more than 24 directions ).
StaticTilePoint WallVectors[8] = {
	{2, 0, 0}, {1, 1, 0}, {0, 2, 0},
	{-1, 2, 0}, {-2, 0, 0}, {-1, -1, 0},
	{0, -2, 0}, {1, -1, 0}
};

StaticTilePoint FireballVectors[24] = {
	{4, 0, 0}, {4, 1, 0}, {3, 2, 0},
	{3, 3, 0}, {2, 3, 0}, {1, 4, 0},
	{0, 4, 0}, {-1, 4, 0}, {-2, 3, 0},
	{-3, 3, 0}, {-3, 2, 0}, {-4, 1, 0},
	{-4, 0, 0}, {-4, -1, 0}, {-3, -2, 0},
	{-3, -3, 0}, {-2, -3, 0}, {-1, -4, 0},
	{0, -4, 0}, {1, -4, 0}, {2, -3, 0},
	{3, -3, 0}, {3, -2, 0}, {4, -1, 0}
};

StaticTilePoint SquareSpellVectors[32] = {
	{4, 0, 0}, {4, 1, 0}, {4, 2, 0}, {4, 3, 0},
	{4, 4, 0}, {3, 4, 0}, {2, 4, 0}, {1, 4, 0},
	{0, 4, 0}, {-1, 4, 0}, {-2, 4, 0}, {-3, 4, 0},
	{-4, 4, 0}, {-4, 3, 0}, {-4, 2, 0}, {-4, 1, 0},
	{-4, 0, 0}, {-4, -1, 0}, {-4, -2, 0}, {-4, -3, 0},
	{-4, -4, 0}, {-3, -4, 0}, {-2, -4, 0}, {-1, -4, 0},
	{0, -4, 0}, {1, -4, 0}, {2, -4, 0}, {3, -4, 0},
	{4, -4, 0}, {4, -3, 0}, {4, -2, 0}, {4, -1, 0}
};

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern hResContext              *spriteRes;             // sprite resource handle
extern hResContext              *schemeRes;             // sprite resource handle
extern ColorTable               identityColors;

/* ===================================================================== *
   Global data
 * ===================================================================== */

SpriteSet                       *spellSprites;         // longsword test sprites
SpellStuff                      *spellBook;

ColorTable                      spellColorMaps[maxSpellColorMaps];
ColorSchemeList                 *spellSchemes;

int32                           loadedColorMaps;

/* ===================================================================== *
   prototypes
 * ===================================================================== */

static void defineEffects();
static void loadMagicData();

/* ===================================================================== *
   code
 * ===================================================================== */

//-----------------------------------------------------------------------
// InitMagic called from main startup code

void initMagic() {
	g_vm->_edpList = new EffectDisplayPrototypeList(maxEffectPrototypes);
	g_vm->_sdpList = new SpellDisplayPrototypeList(maxSpellPrototypes);

	spellBook = new SpellStuff[maxSpells]();

	defineEffects();
	loadMagicData();

	const int colorSchemeSize = 44;
	Common::SeekableReadStream *stream;

	stream = loadResourceToStream(spriteRes, spellSpriteID, "spell sprites");
	spellSprites = new SpriteSet(stream);
	assert(spellSprites);
	delete stream;

	loadedColorMaps = schemeRes->size(spellSpriteID) / colorSchemeSize;

	stream = loadResourceToStream(schemeRes, spellSpriteID, "scheme list");
	spellSchemes = new ColorSchemeList(loadedColorMaps, stream);
	assert(spellSchemes);
	delete stream;
}


void cleanupMagic() {
	g_vm->_activeSpells->cleanup();
	for (int i = 0; i < maxSpells; i++) {
		spellBook[i].killEffects();
	}
	delete[] spellBook;

	g_vm->_sdpList->cleanup();
	g_vm->_edpList->cleanup();

	delete g_vm->_sdpList;
	delete g_vm->_edpList;
}


/* ===================================================================== *
   Effect data
 * ===================================================================== */

//-----------------------------------------------------------------------
// the macros make things more legible than the entire call

// Set up a display effect shape
#define ADD_EFFECT( n, p, s, f, h, b, i ) ( g_vm->_edpList->add( new EffectDisplayPrototype(n,&p,&s,&f,&h,&b,&i )))
// Chain another effect when done
#define SECOND_EFFECT( e, n, p, s, f, h, b, i ) ( g_vm->_edpList->append( new EffectDisplayPrototype(n,&p,&s,&f,&h,&b,&i ),e))

//-----------------------------------------------------------------------

static void defineEffects() {
	int16 i;
	ADD_EFFECT(1,        invisibleSpellPos, invisibleSprites, invisibleSpellSta, ShortTillThere, ThinTillThere, invisibleSpellInit);
	ADD_EFFECT(1,        auraSpellPos, auraSprites, auraSpellSta, ShortTillThere, ThinTillThere, auraSpellInit);
	ADD_EFFECT(1,        projectileSpellPos, projectileSprites, projectileSpellSta, StaticHeight, StaticBreadth, projectileSpellInit);
	ADD_EFFECT(12,       exchangeSpellPos, exchangeSprites, exchangeSpellSta, StaticHeight, StaticBreadth, exchangeSpellInit);
	ADD_EFFECT(36,       boltSpellPos, boltSprites, boltSpellSta, StaticHeight, StaticBreadth, boltSpellInit);
	ADD_EFFECT(24,       coneSpellPos, coneSprites, coneSpellSta, GrowLinear, BulkLinear, coneSpellInit);
	i = ADD_EFFECT(1,      projectileSpellPos, projectileSprites, projectileSpellSta, StaticHeight, StaticBreadth, projectileSpellInit);
	SECOND_EFFECT(i, 24, ballSpellPos, ballSprites, ballSpellSta, ShortTillThere, ThinTillThere, ballSpellInit);
	i = ADD_EFFECT(1,      projectileSpellPos, projectileSprites, projectileSpellSta, StaticHeight, StaticBreadth, projectileSpellInit);
	SECOND_EFFECT(i, 32, squareSpellPos, squareSprites, squareSpellSta, StaticHeight, StaticBreadth, squareSpellInit);
	ADD_EFFECT(24,       waveSpellPos, waveSprites, waveSpellSta, GrowLinear, BulkLinear, waveSpellInit);
	i = ADD_EFFECT(1,      projectileSpellPos, projectileSprites, projectileSpellSta, StaticHeight, StaticBreadth, projectileSpellInit);
	SECOND_EFFECT(i, 24,  stormSpellPos, stormSprites, stormSpellSta, ShortTillThere, ThinTillThere, stormSpellInit);
	i = ADD_EFFECT(1,      projectileSpellPos, projectileSprites, projectileSpellSta, StaticHeight, StaticBreadth, projectileSpellInit);
	SECOND_EFFECT(i, 1,   glowSpellPos, auraSprites, auraSpellSta, ShortTillThere, ThinTillThere, glowSpellInit);
	ADD_EFFECT(1,        glowSpellPos, auraSprites, auraSpellSta, ShortTillThere, ThinTillThere, glowSpellInit);
	ADD_EFFECT(20,       beamSpellPos, beamSprites, beamSpellSta, StaticHeight, StaticBreadth, beamSpellInit);
	ADD_EFFECT(8,       wallSpellPos, wallSprites, wallSpellSta, StaticHeight, StaticBreadth, wallSpellInit);

}

//-----------------------------------------------------------------------
#define ADD_SHOW( e, a, b, c, d, f, g, m, i, s, n ) ( g_vm->_sdpList->add( new SpellDisplayPrototype( e, a, b, c, d, f, g, m, i, s, n )))

//-----------------------------------------------------------------------
// loadMagicData : reads magic related data from the resource file

static void loadMagicData() {
	int16           i;
	hResContext     *spellRes;

	//  Get spell definitions
	spellRes =  auxResFile->newContext(
	                MKTAG('S', 'P', 'E', 'L'),
	                "spell resources");
	if (spellRes == nullptr || !spellRes->_valid)
		error("Error accessing spell resource group.\n");
	i = 1;
	ADD_SHOW(eAreaInvisible, 0, 0, 0, 0, diFlagInc, ecFlagNone,  30, MKTAG('S', 'T', 'A', 0), 23, 24);

	spellBook[0].setManaType(sManaIDSkill);

	while (spellRes->size(
	            MKTAG('I', 'N', 'F', i)) > 0) {
		ResourceSpellItem *rsi =
		    (ResourceSpellItem *)LoadResource(
		        spellRes,
		        MKTAG('I', 'N', 'F', i),
		        "spell");

		if (rsi == nullptr)
			error("Unable to load data for spell %d", i);

		spellBook[rsi->spell].setupFromResource(rsi);
		g_vm->_sdpList->add(new SpellDisplayPrototype(rsi));

		free(rsi);
		i++;
	}
	assert(i > 1);

	// get spell effects
	i = 0;
	while (spellRes->size(
	            MKTAG('E', 'F', 'F', i)) > 0) {
		ResourceSpellEffect *rse =
		    (ResourceSpellEffect *)LoadResource(
		        spellRes,
		        MKTAG('E', 'F', 'F', i),
		        "spell effect");

		if (rse == nullptr)
			error("Unable to load effects for spell %d", i);

		if (rse->spell)
			spellBook[rse->spell].addEffect(rse);

		free(rse);
		i++;
	}
	assert(i > 1);


	// get spell color maps
	memcpy(spellColorMaps[0], identityColors, sizeof(ColorTable));
	auxResFile->disposeContext(spellRes);
}

} // end of namespace Saga2
