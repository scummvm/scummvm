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

extern ColorSchemeList  *spellSchemes;

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
	_nodeCount = nodes;
	_location = newLocation;
	_spriteno = newSpriteno;
	_status  = newStatus;
	_height  = newHeight;
	_breadth = newBreadth;
	_init    = newInit;
	_next = nullptr;
	_ID = spellNone;
}

/* ===================================================================== *
   EffectDisplayPrototypeList implementation
 * ===================================================================== */

EffectDisplayPrototypeList::EffectDisplayPrototypeList(int32 c) {
	_count = 0;
	_maxCount = 0;
	_effects = new pEffectDisplayPrototype[c]();
	for (int i = 0; i < c; i++)
		_effects[i] = nullptr;
	assert(_effects);
	if (_effects) _maxCount = c;
}

EffectDisplayPrototypeList::~EffectDisplayPrototypeList() {
	if (_maxCount && _effects)
		delete[] _effects;
	_maxCount = 0;
	_effects = nullptr;
}

int32 EffectDisplayPrototypeList::add(EffectDisplayPrototype *edp) {
	assert(_count < _maxCount);
	edp->setID(_count);
	_effects[_count++] = edp;
	return _count - 1;
}

void EffectDisplayPrototypeList::cleanup() {
	if (_maxCount && _effects)
		for (int i = 0; i < _maxCount; i++)
			if (_effects[i]) {
				delete _effects[i];
				_effects[i] = nullptr;
			}
	_maxCount = 0;
}

void EffectDisplayPrototypeList::append(EffectDisplayPrototype *nedp, int32 acount) {
	assert(acount < _maxCount);
	EffectDisplayPrototype *edp = _effects[acount];
	while (edp->_next) edp = edp->_next;
	edp->_next = nedp;
}

EffectDisplayPrototype *EffectDisplayPrototypeList::operator[](EffectID e) {
	assert(e < _maxCount);
	return _effects[e];
}

/* ===================================================================== *
   SpellDisplayPrototype implementation
 * ===================================================================== */

SpellDisplayPrototype::SpellDisplayPrototype(
    EffectID e, int32 e1, int32 e2, int32 e3, int32 e4,
    effectDirectionInit sc, effectCollisionCont cc, SpellAge sa,
    uint32 spID, uint8 sco, uint8) {
	_effect = e;      // Effect ID
	_effParm1 = e1;    //   effect setting 1
	_effParm2 = e2;    //   effect setting 1
	_effParm3 = e3;    //   effect setting 1
	_effParm4 = e4;    //   effect setting 1

	_scatter = sc;     // direction init mode
	_elasticity = cc;  // collision flags

	_maxAge = sa;      // auto self-destruct age
	_primarySpriteID = spID; // RES_ID( x, y, z, 0 ) to get sprites
	_primarySpriteNo = sco; // sprites available
	_secondarySpriteID = spID; // RES_ID( x, y, z, 0 ) to get sprites
	_secondarySpriteNo = sco; // sprites available
	//_effCount=eco;      // effectrons to allocate

	_ID = spellNone;
	_implementAge = 0;
}

SpellDisplayPrototype *SpellDisplayPrototypeList::operator[](SpellID s) {
	assert(s >= 0 && s < _count);
	return _spells[s];
}

void SpellDisplayPrototype::getColorTranslation(ColorTable map, Effectron *e) {
	int32 i = _colorMap[whichColorMap(_effect, e)];
	i = MAX<int32>(0, MIN(loadedColorMaps, i));
	buildColorTable(map, spellSchemes->_schemes[i]->bank, 11);
}

/* ===================================================================== *
   SpellDisplayPrototypeList implementation
 * ===================================================================== */

void SpellDisplayPrototypeList::init() {
	// originally this was going to load data from the resfile
	// that plan has been cancelled
}

void SpellDisplayPrototypeList::cleanup() {
	if (_spells) {
		for (int i = 0; i < _maxCount; i++)
			if (_spells[i]) {
				delete _spells[i];
				_spells[i] = nullptr;
			}
		delete[] _spells;
		_spells = nullptr;
		_maxCount = 0;
	}
}

SpellDisplayPrototypeList::SpellDisplayPrototypeList(uint16 s) {
	_count = 0;
	_maxCount = 0;
	_spells = new pSpellDisplayPrototype[s]();
	for (int i = 0; i < s; i++)
		_spells[i] = nullptr;
	assert(_spells);
	if (_spells) _maxCount = s;
}

SpellDisplayPrototypeList::~SpellDisplayPrototypeList() {
	if (_maxCount && _spells)
		delete[] _spells;
	_spells = nullptr;
}

int32 SpellDisplayPrototypeList::add(SpellDisplayPrototype *sdp) {
	assert(_count < _maxCount);
	sdp->setID((SpellID) _count);
	_spells[_count++] = sdp;
	return _count;
}

/* ===================================================================== *
   SpellDisplayList implementation
 * ===================================================================== */

SpellDisplayList::SpellDisplayList(uint16 s) {
	_count = 0;
	_maxCount = 0;
	_spells = new pSpellInstance[s]();
	for (int i = 0; i < s; i++)
		_spells[i] = nullptr;
	if (_spells) _maxCount = s;
	init();
}

SpellDisplayList::~SpellDisplayList() {
	cleanup();
}

void SpellDisplayList::init() {
	_count = 0;
}

void SpellDisplayList::cleanup() {
	if (_maxCount && _spells)
		delete[] _spells;
	_spells = nullptr;
}

void SpellDisplayList::add(SpellInstance *newSpell) {
	assert(newSpell);
	if (_count < _maxCount)
		_spells[_count++] = newSpell;
}

void SpellDisplayList::buildList() {
	if (_count)
		for (int16 i = 0; i < _count; i++)   // check all active _spells
			if (!_spells[i]->buildList()) {   // update
				tidyKill(i--);              // that spell is done
			}
}

void SpellDisplayList::updateStates(int32 deltaTime) {
	if (_count)
		for (int16 i = 0; i < _count; i++)
			_spells[i]->updateStates(deltaTime);
}

void SpellDisplayList::tidyKill(uint16 spellNo) {
	assert(_count);
	if (_spells[spellNo]) {
		delete _spells[spellNo];
		_spells[spellNo] = nullptr;
	}
	if (spellNo < _count--) {
		for (uint16 i = spellNo; i <= _count; i++)
			_spells[i] = _spells[i + 1];
		_spells[_count + 1] = nullptr;
	}
}

} // end of namespace Saga2
