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

#include "saga2/std.h"
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

static void loadWeaponData(void);

ProtoEffect *createNewProtoEffect(ResourceItemEffect *rie) {
	ProtoEffect     *pe = NULL;

	switch (rie->effectGroup) {
	case effectNone     :
		return NULL;

	case effectAttrib   :
		pe = new   ProtoEnchantment(
		         makeEnchantmentID(
		             rie->effectGroup,
		             rie->effectType,
		             rie->baseDamage),
		         rie->reserved0,
		         rie->reserved1);
		break;
	case effectResist   :
	case effectImmune   :
	case effectOthers   :
	case effectNonActor :
		pe = new   ProtoEnchantment(
		         makeEnchantmentID(
		             rie->effectGroup,
		             rie->effectType,
		             rie->skillDamage),
		         rie->reserved0,
		         rie->reserved1);
		break;
	case effectDamage   :
		pe = new   ProtoDamage(
		         rie->baseDice,
		         rie->diceSides ? rie->diceSides : 6,
		         rie->skillDice,
		         rie->baseDamage,
		         (effectDamageTypes) rie->effectType,
		         0,
		         rie->targeting & spellTargCaster,
		         rie->skillDamage);
		break;
	case effectDrains   :
		pe = new   ProtoDrainage(
		         rie->baseDice,
		         rie->diceSides ? rie->diceSides : 6,
		         rie->skillDice,
		         rie->baseDamage,
		         (effectDrainsTypes) rie->effectType,
		         0,
		         rie->targeting & spellTargCaster);
		break;

	case effectPoison   :
		pe = new   ProtoEnchantment(
		         makeEnchantmentID(rie->baseDamage),      // poison
		         rie->reserved0,
		         rie->reserved1);

	case effectTAG      :
		pe = new   ProtoTAGEffect(
		         (effectTAGTypes) rie->effectType,
		         rie->skillDamage,
		         rie->baseDamage);
		break;
	case effectLocation :
		pe = new   ProtoLocationEffect(
		         (effectLocationTypes) rie->effectType,
		         rie->baseDamage);
		break;
	case effectSpecial  :
		pe = new   ProtoSpecialEffect(
		         SagaSpellCall,
		         rie->baseDamage);
		break;
	}

	if (pe == NULL)
		error("failed to alloc protoEffect");

	return pe;
}

//-----------------------------------------------------------------------
// InitMagic called from main startup code

void initWeapons(void) {
	loadWeaponData();
}

//-----------------------------------------------------------------------

void cleanupWeapons(void) {
	for (int i = 0; i < kMaxWeapons; i++)
		g_vm->_weaponRack[i].killEffects();
}

//-----------------------------------------------------------------------

WeaponStuff &getWeapon(weaponID i) {
	if (i < g_vm->_loadedWeapons)
		return g_vm->_weaponRack[i];
	return g_vm->_weaponRack[nullWeapon];
}

//-----------------------------------------------------------------------

GameObject *getShieldItem(GameObject *defender) {
	assert(isActor(defender));
	Actor       *a = (Actor *) defender;
	GameObject  *obj;

	a->defensiveObject(&obj);
	return obj;
}

/* ===================================================================== *
   WeaponProtoEffect member functions
 * ===================================================================== */

WeaponProtoEffect::~WeaponProtoEffect(void) {
	if (effect != NULL)
		delete effect;
}

void WeaponProtoEffect::implement(
    Actor       *enactor,
    GameObject  *target,
    GameObject *,
    uint8) {
	SpellTarget     targ(target);

	if (effect != NULL)
		effect->implement(enactor, &targ);
}

/* ===================================================================== *
   WeaponStrikeEffect member functions
 * ===================================================================== */

//-----------------------------------------------------------------------

void WeaponStrikeEffect::implement(
    Actor       *enactor,
    GameObject  *target,
    GameObject  *strikingObj,
    uint8       strength) {
	assert(isActor(enactor));
	assert(isObject(target) || isActor(target));
	assert(isObject(strikingObj) || isActor(strikingObj));

	int8        totalDice,
	            totalBase;

	totalDice = dice + strength * skillDice;
	totalBase = base + strength * skillBase;

	target->acceptDamage(
	    enactor->thisID(),
	    totalBase,
	    type,
	    totalDice,
	    sides);
}

//-----------------------------------------------------------------------

WeaponStuff::WeaponStuff() {
	effects = NULL;
	master = nullWeapon;
}

//-----------------------------------------------------------------------

WeaponStuff::~WeaponStuff() {
	while (effects != NULL) {
		WeaponEffect    *curEffect = effects;

		effects = effects->next;
		delete curEffect;
	}
	master = nullWeapon;
}

//-----------------------------------------------------------------------

void WeaponStuff::killEffects(void) {
	while (effects != NULL) {
		WeaponEffect    *curEffect = effects;

		effects = effects->next;
		delete curEffect;
	}
}

//-----------------------------------------------------------------------

void WeaponStuff::addEffect(WeaponEffect *we) {
	WeaponEffect *e = effects;
	if (effects) {
		while (e->next) e = e->next;
		e->next = we;
	} else {
		effects = we;
	}
}

//-----------------------------------------------------------------------

void WeaponStuff::addEffect(ResourceItemEffect *rie) {
	WeaponEffect *we;
	assert(rie);
	assert(rie && rie->item == master);

	if (rie->effectGroup == effectStrike) {
		we = new   WeaponStrikeEffect(
		         (effectDamageTypes)rie->effectType,
		         rie->baseDice,
		         rie->diceSides != 0 ? rie->diceSides : 6,
		         rie->skillDice,
		         rie->baseDamage,
		         rie->skillDamage);
	} else
		we = new   WeaponProtoEffect(rie);

	if (we == NULL)
		error("failed to alloc weapon effect");

	if (effects == NULL)
		effects = we;
	else {
		WeaponEffect *tail;
		for (tail = effects; tail->next; tail = tail->next) ;
		tail->next = we;
	}
}

//-----------------------------------------------------------------------

void WeaponStuff::implement(
    Actor       *enactor,
    GameObject  *target,
    GameObject  *strikingObj,
    uint8       strength) {
	WeaponEffect        *we;

	for (we = effects; we != NULL; we = we->next)
		we->implement(enactor, target, strikingObj, strength);
}

//-----------------------------------------------------------------------

static void loadWeaponData(void) {
	int16           i;
	hResContext     *spellRes;

	//  Get spell definitions
	spellRes =  auxResFile->newContext(
	                MKTAG('I', 'T', 'E', 'M'),
	                "weapon resources");
	if (spellRes == NULL || !spellRes->_valid)
		error("Error accessing weapon resource group.");

	// get spell effects
	i = 0;
	while (spellRes->size(
	            MKTAG('E', 'F', 'F', i)) > 0) {
		ResourceItemEffect *rie =
		    (ResourceItemEffect *)LoadResource(
		        spellRes,
		        MKTAG('E', 'F', 'F', i),
		        "weapon effect");

		if (rie == NULL)
			error("Unable to load weapon effect %d", i);

		if (rie->item) {
			g_vm->_weaponRack[rie->item].setID(rie->item);
			g_vm->_weaponRack[rie->item].addEffect(rie);
		}

		RDisposePtr(rie);
		i++;
	}
	g_vm->_loadedWeapons = i;
	assert(i > 1);

	auxResFile->disposeContext(spellRes);
}

} // end of namespace Saga2
