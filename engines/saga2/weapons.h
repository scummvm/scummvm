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

#ifndef SAGA2_WEAPONS_H
#define SAGA2_WEAPONS_H

#include "saga2/spelshow.h"
#include "saga2/spells.h"
#include "saga2/dice.h"
#include "saga2/effects.h"
#include "saga2/actor.h"
#include "saga2/spellbuk.h"

namespace Saga2 {

ProtoEffect *createNewProtoEffect(ResourceItemEffect *rie);

//-----------------------------------------------------------------------

class WeaponEffect {
public:
	WeaponEffect *next;                      // pointer to additional effects

	WeaponEffect(void) : next(NULL) {}
	virtual ~WeaponEffect(void) {}
	virtual void implement(
	    Actor       *enactor,
	    GameObject  *target,
	    GameObject  *strikingObj,
	    uint8       strength) = 0;
};

//-----------------------------------------------------------------------

class WeaponProtoEffect : public WeaponEffect {
	ProtoEffect     *effect;

public:
	WeaponProtoEffect(ResourceItemEffect *rie) :
		effect(createNewProtoEffect(rie)) {
	}
	~WeaponProtoEffect(void) {
		if (effect != NULL) delete effect;
	}

	void implement(
	    Actor       *enactor,
	    GameObject  *target,
	    GameObject  *strikingObj,
	    uint8       strength);
};

//-----------------------------------------------------------------------

class WeaponStrikeEffect : public WeaponEffect {
	effectDamageTypes   type;               // damage type
	int8                dice,               // # of dice to roll
	                    sides,              // # of sides on dice
	                    skillDice,          // multiply additional dice
	                    base,               // absolute damage amount
	                    skillBase;

public:
	WeaponStrikeEffect(
	    effectDamageTypes t,
	    int8 d,
	    int8 s,
	    int8 sd,
	    int8 b,
	    int8 sb) :
		type(t),
		dice(d),
		sides(s),
		skillDice(sd),
		base(b),
		skillBase(sb) {
	}

	void implement(
	    Actor       *enactor,
	    GameObject  *target,
	    GameObject  *strikingObj,
	    uint8       strength);
};

//-----------------------------------------------------------------------

class WeaponStuff {
	weaponID            master;             // index in array
	WeaponEffect        *effects;           // the effects of this weapon

public:
	WeaponStuff();
	~WeaponStuff();
	void setID(weaponID id) {
		master = id;
	}
	void addEffect(WeaponEffect *we);
	void addEffect(ResourceItemEffect *rse);
	void killEffects(void);
	void implement(
	    Actor       *enactor,
	    GameObject  *target,
	    GameObject  *strikingObj,
	    uint8       strength);
};

//-----------------------------------------------------------------------


void initWeapons(void);
void cleanupWeapons(void);

WeaponStuff &getWeapon(weaponID i);

GameObject *getShieldItem(GameObject *defender);

} // End of namespace Saga2

#endif
