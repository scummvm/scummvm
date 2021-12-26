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
#include "saga2/weapons.h"
#include "saga2/spelshow.h"
#include "saga2/spellbuk.h"
#include "saga2/actor.h"
#include "saga2/spellio.h"
#include "saga2/hresmgr.h"
#include "saga2/combat.h"

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

static void loadWeaponData();

ProtoEffect *createNewProtoEffect(Common::SeekableReadStream *stream) {
	ProtoEffect *pe = nullptr;

	/* int16 item = */stream->readSint16LE();	// spell ID
	int16 effectGroup = stream->readSint16LE();	// effect group

	int16 effectType = stream->readSint16LE();	// effect ID
	int16 targeting = stream->readSint16LE();	// targeting
	int16 baseDice = stream->readSint16LE();	// base dice
	int16 skillDice = stream->readSint16LE();	// skill dice
	int16 baseDamage = stream->readSint16LE();	// attrib change
	int16 skillDamage = stream->readSint16LE();	// attrib change
	int16 reserved0 = stream->readSint16LE();	// min enchant
	int16 reserved1 = stream->readSint16LE();	// min enchant
	int16 diceSides = stream->readSint16LE();

	if (diceSides == 0)
		diceSides = 6;

	switch (effectGroup) {
	case effectNone:
		return nullptr;

	case effectAttrib:
		pe = new ProtoEnchantment(makeEnchantmentID(effectGroup, effectType, baseDamage), reserved0, reserved1);
		break;

	case effectResist:
	case effectImmune:
	case effectOthers:
	case effectNonActor:
		pe = new ProtoEnchantment(makeEnchantmentID(effectGroup, effectType, skillDamage), reserved0, reserved1);
		break;

	case effectDamage:
		pe = new ProtoDamage(baseDice, diceSides, skillDice, baseDamage,
					(effectDamageTypes)effectType, 0, targeting & spellTargCaster, skillDamage);
		break;

	case effectDrains:
		pe = new ProtoDrainage(baseDice, diceSides, skillDice, baseDamage,
					(effectDrainsTypes)effectType, 0, targeting & spellTargCaster);
		break;

	case effectPoison:
		pe = new ProtoEnchantment(makeEnchantmentID(baseDamage),      // poison
					reserved0, reserved1);
		break;

	case effectTAG:
		pe = new ProtoTAGEffect((effectTAGTypes)effectType, skillDamage, baseDamage);
		break;

	case effectLocation:
		pe = new ProtoLocationEffect((effectLocationTypes)effectType, baseDamage);
		break;

	case effectSpecial:
		pe = new ProtoSpecialEffect(SagaSpellCall, baseDamage);
		break;
	}

	if (pe == nullptr)
		error("failed to alloc protoEffect");

	return pe;
}

void initWeapons() {
	loadWeaponData();
}

void cleanupWeapons() {
	for (int i = 0; i < kMaxWeapons; i++)
		g_vm->_weaponRack[i].killEffects();
}

WeaponStuff &getWeapon(weaponID i) {
	if (i < g_vm->_loadedWeapons)
		return g_vm->_weaponRack[i];
	return g_vm->_weaponRack[kNullWeapon];
}

GameObject *getShieldItem(GameObject *defender) {
	assert(isActor(defender));
	Actor *a = (Actor *) defender;
	GameObject *obj;

	a->defensiveObject(&obj);
	return obj;
}

/* ===================================================================== *
   WeaponProtoEffect member functions
 * ===================================================================== */
WeaponProtoEffect::~WeaponProtoEffect() {
	if (_effect != nullptr)
		delete _effect;
}

void WeaponProtoEffect::implement(Actor *enactor, GameObject *target, GameObject *, uint8) {
	SpellTarget targ(target);

	if (_effect != nullptr)
		_effect->implement(enactor, &targ);
}

/* ===================================================================== *
   WeaponStrikeEffect member functions
 * ===================================================================== */
void WeaponStrikeEffect::implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 strength) {
	assert(isActor(enactor));
	assert(isObject(target) || isActor(target));
	assert(isObject(strikingObj) || isActor(strikingObj));

	int8 totalDice, totalBase;

	totalDice = _dice + strength * _skillDice;
	totalBase = _base + strength * _skillBase;

	target->acceptDamage(enactor->thisID(), totalBase, _type, totalDice, _sides);
}

WeaponStuff::WeaponStuff() {
	_effects = nullptr;
	_master = kNullWeapon;
}

WeaponStuff::~WeaponStuff() {
	while (_effects != nullptr) {
		WeaponEffect *curEffect = _effects;

		_effects = _effects->_next;
		delete curEffect;
	}
	_master = kNullWeapon;
}

void WeaponStuff::killEffects() {
	while (_effects != nullptr) {
		WeaponEffect *curEffect = _effects;

		_effects = _effects->_next;
		delete curEffect;
	}
}

void WeaponStuff::addEffect(WeaponEffect *we) {
	WeaponEffect *e = _effects;
	if (_effects) {
		while (e->_next)
			e = e->_next;
		e->_next = we;
	} else {
		_effects = we;
	}
}

void WeaponStuff::addEffect(Common::SeekableReadStream *stream) {
	WeaponEffect *we;

	/*int16 item = */stream->readSint16LE();		// spell ID
	int16 effectGroup = stream->readSint16LE();	// effect group

	if (effectGroup == effectStrike) {
		effectDamageTypes effectType = (effectDamageTypes)stream->readSint16LE();	// effect ID
		/*int16 targeting = */stream->readSint16LE();	// targeting
		int16 baseDice = stream->readSint16LE();	// base dice
		int16 skillDice = stream->readSint16LE();	// skill dice
		int16 baseDamage = stream->readSint16LE();	// attrib change
		int16 skillDamage = stream->readSint16LE();	// attrib change
		/*int16 reserved0 = */stream->readSint16LE();	// min enchant
		/* int16 reserved1 = */stream->readSint16LE();	// min enchant
		int16 diceSides = stream->readSint16LE();

		if (diceSides == 0)
			diceSides = 6;

		we = new WeaponStrikeEffect(effectType, baseDice, diceSides, skillDice, baseDamage, skillDamage);
	} else {
		stream->seek(0);
		we = new WeaponProtoEffect(stream);
	}

	if (we == nullptr)
		error("failed to alloc weapon effect");

	if (_effects == nullptr)
		_effects = we;
	else {
		WeaponEffect *tail;
		for (tail = _effects; tail->_next; tail = tail->_next)
			;
		tail->_next = we;
	}
}

void WeaponStuff::implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 strength) {
	for (WeaponEffect *we = _effects; we != nullptr; we = we->_next)
		we->implement(enactor, target, strikingObj, strength);
}

//-----------------------------------------------------------------------

static void loadWeaponData() {
	hResContext *spellRes = auxResFile->newContext(MKTAG('I', 'T', 'E', 'M'), "weapon resources");
	if (spellRes == nullptr || !spellRes->_valid)
		error("Error accessing weapon resource group.");

	debugC(1, kDebugLoading, "Loading Weapon Data");

	// get spell effects
	int16 i = 0;
	while (spellRes->size(MKTAG('E', 'F', 'F', i)) > 0) {
		Common::SeekableReadStream *stream = loadResourceToStream(spellRes, MKTAG('E', 'F', 'F', i), "weapon effect");

		if (stream == nullptr)
			error("Unable to load weapon effect %d", i);

		int16 id = stream->readSint16LE();
		stream->seek(0);

		if (id) {
			g_vm->_weaponRack[id].setID(id);
			g_vm->_weaponRack[id].addEffect(stream);
		}

		delete stream;

		i++;
	}
	g_vm->_loadedWeapons = i;
	assert(i > 1);

	debugC(1, kDebugLoading, "Loading Weapon Data, loaded %d entries", i);

	auxResFile->disposeContext(spellRes);
}

} // end of namespace Saga2
