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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/rmemfta.h"
#include "saga2/dispnode.h"
#include "saga2/spellbuk.h"
#include "saga2/spelshow.h"

namespace Saga2 {

// ------------------------------------------------------------------
// This file contains the member functions for:
//   EffectDisplayPrototype
//   EffectDisplayPrototypeList
//   SpellDisplayPrototype
//   SpellDisplayPrototypeList
//   SpellDisplayList

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern ColorScheme  **spellSchemes;

extern ColorTable   spellColorMaps[];
extern int32        loadedColorMaps;

/* ===================================================================== *
   EffectDisplayPrototype implementation
 * ===================================================================== */

EffectDisplayPrototype::EffectDisplayPrototype(
    int16                   nodes,
    SpellLocationFunction   *newLocation,
    SpellSpritationFunction *newSpriteno,
    SpellStatusFunction     *newStatus,
    SpellHeightFunction     *newHeight,
    SpellBreadthFunction    *newBreadth,
    SpellInitFunction       *newInit) {
	nodeCount = nodes;
	location = newLocation;
	spriteno = newSpriteno;
	status  = newStatus;
	height  = newHeight;
	breadth = newBreadth;
	init    = newInit;
	next = NULL;
}

/* ===================================================================== *
   EffectDisplayPrototypeList implementation
 * ===================================================================== */

EffectDisplayPrototypeList::EffectDisplayPrototypeList(int32 c) {
	count = 0;
	maxCount = 0;
	effects = (pEffectDisplayPrototype *) TALLOC(sizeof(pEffectDisplayPrototype) * c, memSpells);
	for (int i = 0; i < c; i++)
		effects[i] = NULL;
	assert(effects);
	if (effects) maxCount = c;
}

EffectDisplayPrototypeList::~EffectDisplayPrototypeList() {
	if (maxCount && effects)
		for (int i = 0; i < maxCount; i++)
			if (effects[i]) {
				delete effects[i];
				effects[i] = NULL;
			}
	maxCount = 0;
	if (effects)
		delete effects;
	effects = NULL;
}

int32 EffectDisplayPrototypeList::add(EffectDisplayPrototype *edp) {
	assert(count < maxCount);
	edp->setID(count);
	effects[count++] = edp;
	return count - 1;
}

void EffectDisplayPrototypeList::cleanup(void) {
	if (maxCount && effects)
		for (int i = 0; i < maxCount; i++)
			if (effects[i]) {
				delete effects[i];
				effects[i] = NULL;
			}
	maxCount = 0;
}

void EffectDisplayPrototypeList::append(EffectDisplayPrototype *nedp, int32 acount) {
	assert(acount < maxCount);
	EffectDisplayPrototype *edp = effects[acount];
	while (edp->next) edp = edp->next;
	edp->next = nedp;
}

EffectDisplayPrototype *EffectDisplayPrototypeList::operator[](EffectID e) {
	assert(e >= 0 && e < maxCount);
	return effects[e];
}

/* ===================================================================== *
   SpellDisplayPrototype implementation
 * ===================================================================== */

SpellDisplayPrototype::SpellDisplayPrototype(
    EffectID e, int32 e1, int32 e2, int32 e3, int32 e4,
    effectDirectionInit sc, effectCollisionCont cc, SpellAge sa,
    uint32 spID, uint8 sco, uint8) {
	effect = e;      // Effect ID
	effParm1 = e1;    //   effect setting 1
	effParm2 = e2;    //   effect setting 1
	effParm3 = e3;    //   effect setting 1
	effParm4 = e4;    //   effect setting 1

	scatter = sc;     // direction init mode
	elasticity = cc;  // collision flags

	maxAge = sa;      // auto self-destruct age
	primarySpriteID = spID; // RES_ID( x, y, z, 0 ) to get sprites
	primarySpriteNo = sco; // sprites available
	secondarySpriteID = spID; // RES_ID( x, y, z, 0 ) to get sprites
	secondarySpriteNo = sco; // sprites available
	//effCount=eco;      // effectrons to allocate
}

SpellDisplayPrototype *SpellDisplayPrototypeList::operator[](SpellID s) {
	assert(s >= 0 && s < count);
	return spells[s];
}

void SpellDisplayPrototype::getColorTranslation(ColorTable map, Effectron *e) {
	int32 i = colorMap[whichColorMap(effect, e)];
	i = MAX(0, MIN(loadedColorMaps, i));
	buildColorTable(map,
	                (*spellSchemes)[ i ].bank,
	                11);
}

/* ===================================================================== *
   SpellDisplayPrototypeList implementation
 * ===================================================================== */

void SpellDisplayPrototypeList::init(void) {
	// originally this was going to load data from the resfile
	// that plan has been cancelled
}

void SpellDisplayPrototypeList::cleanup(void) {
	if (spells) {
		for (int i = 0; i < maxCount; i++)
			if (spells[i]) {
				delete spells[i];
				spells[i] = NULL;
			}
		delete spells;
		spells = NULL;
		maxCount = 0;
	}
}

SpellDisplayPrototypeList::SpellDisplayPrototypeList(uint16 s) {
	count = 0;
	maxCount = 0;
	spells = (pSpellDisplayPrototype *) TALLOC(sizeof(pSpellDisplayPrototype) * s, memSpells);
	for (int i = 0; i < s; i++)
		spells[i] = NULL;
	assert(spells);
	if (spells) maxCount = s;
}

SpellDisplayPrototypeList::~SpellDisplayPrototypeList() {
	if (maxCount && spells)
		for (int i = 0; i < maxCount; i++)
			if (spells[i]) {
				delete spells[i];
				spells[i] = NULL;
			}
	if (spells)
		delete spells;
	spells = NULL;
}

int32 SpellDisplayPrototypeList::add(SpellDisplayPrototype *sdp) {
	assert(count < maxCount);
	sdp->setID((SpellID) count);
	spells[count++] = sdp;
	return count;
}

/* ===================================================================== *
   SpellDisplayList implementation
 * ===================================================================== */

SpellDisplayList::SpellDisplayList(uint16 s) {
	count = 0;
	maxCount = 0;
	spells = (pSpellInstance *) TALLOC(sizeof(pSpellInstance) * s, memSpells);
	for (int i = 0; i < s; i++)
		spells[i] = NULL;
	if (spells) maxCount = s;
	init();
}

SpellDisplayList::~SpellDisplayList() {
	cleanup();
}

void SpellDisplayList::init(void) {
	count = 0;
}

void SpellDisplayList::cleanup(void) {
	if (maxCount && spells)
		for (int i = 0; i < maxCount; i++)
			if (spells[i]) {
				delete spells[i];
				spells[i] = NULL;
			}
	if (spells)
		delete spells;
	spells = NULL;
}

void SpellDisplayList::add(SpellInstance *newSpell) {
	assert(newSpell);
	if (count < maxCount)
		spells[count++] = newSpell;
}

void SpellDisplayList::buildList(void) {
	if (count)
		for (int16 i = 0; i < count; i++)   // check all active spells
			if (!spells[i]->buildList()) {   // update
				tidyKill(i--);              // that spell is done
			}
}

void SpellDisplayList::updateStates(int32 deltaTime) {
	if (count)
		for (int16 i = 0; i < count; i++)
			spells[i]->updateStates(deltaTime);
}

void SpellDisplayList::tidyKill(uint16 spellNo) {
	assert(count);
	if (spells[spellNo]) {
		delete spells[spellNo];
		spells[spellNo] = NULL;
	}
	if (spellNo < count--) {
		for (uint16 i = spellNo; i <= count; i++)
			spells[i] = spells[i + 1];
		spells[count + 1] = NULL;
	}
}

} // end of namespace Saga2
