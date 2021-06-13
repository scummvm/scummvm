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

namespace Saga2 {

struct ResourceItemEffect;

class Actor;
class GameObject;
class ProtoEffect;

ProtoEffect *createNewProtoEffect(Common::SeekableReadStream *stream);

//-----------------------------------------------------------------------

class WeaponEffect {
public:
	WeaponEffect *next;                      // pointer to additional effects

	WeaponEffect(void) : next(NULL) {}
	virtual ~WeaponEffect(void) {}
	virtual void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8  strength) = 0;
};

//-----------------------------------------------------------------------

class WeaponProtoEffect : public WeaponEffect {
	ProtoEffect *effect;

public:
	WeaponProtoEffect(Common::SeekableReadStream *stream) : effect(createNewProtoEffect(stream)) {
	}
	~WeaponProtoEffect(void);

	void implement(Actor *enactor, GameObject*target, GameObject *strikingObj, uint8 strength);
};

//-----------------------------------------------------------------------

class WeaponStrikeEffect : public WeaponEffect {
	effectDamageTypes type;	// damage type
	int8 dice;				// # of dice to roll
	int8 sides;				// # of sides on dice
	int8 skillDice;			// multiply additional dice
	int8 base;				// absolute damage amount
	int8 skillBase;

public:
	WeaponStrikeEffect(effectDamageTypes t, int8 d, int8 s, int8 sd, int8 b, int8 sb) :
		type(t), dice(d), sides(s), skillDice(sd), base(b), skillBase(sb) {
	}

	void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 trength);
};

//-----------------------------------------------------------------------

class WeaponStuff {
	weaponID master;             // index in array
	WeaponEffect *effects;           // the effects of this weapon

public:
	WeaponStuff();
	~WeaponStuff();
	void setID(weaponID id) {
		master = id;
	}
	void addEffect(WeaponEffect *we);
	void addEffect(Common::SeekableReadStream *stream);
	void killEffects(void);
	void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 strength);
};

//-----------------------------------------------------------------------


void initWeapons(void);
void cleanupWeapons(void);

WeaponStuff &getWeapon(weaponID i);

GameObject *getShieldItem(GameObject *defender);

} // End of namespace Saga2

#endif
