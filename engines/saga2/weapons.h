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
	WeaponEffect *_next;                      // pointer to additional effects

	WeaponEffect() : _next(NULL) {}
	virtual ~WeaponEffect() {}
	virtual void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8  strength) = 0;
};

//-----------------------------------------------------------------------

class WeaponProtoEffect : public WeaponEffect {
	ProtoEffect *_effect;

public:
	WeaponProtoEffect(Common::SeekableReadStream *stream) : _effect(createNewProtoEffect(stream)) {
	}
	~WeaponProtoEffect();

	void implement(Actor *enactor, GameObject*target, GameObject *strikingObj, uint8 strength);
};

//-----------------------------------------------------------------------

class WeaponStrikeEffect : public WeaponEffect {
	effectDamageTypes _type;// damage type
	int8 _dice;				// # of dice to roll
	int8 _sides;			// # of sides on dice
	int8 _skillDice;		// multiply additional dice
	int8 _base;				// absolute damage amount
	int8 _skillBase;

public:
	WeaponStrikeEffect(effectDamageTypes t, int8 d, int8 s, int8 sd, int8 b, int8 sb) :
		_type(t), _dice(d), _sides(s), _skillDice(sd), _base(b), _skillBase(sb) {
	}

	void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 trength);
};

//-----------------------------------------------------------------------

class WeaponStuff {
	weaponID _master;             // index in array
	WeaponEffect *_effects;           // the effects of this weapon

public:
	WeaponStuff();
	~WeaponStuff();
	void setID(weaponID id) {
		_master = id;
	}
	void addEffect(WeaponEffect *we);
	void addEffect(Common::SeekableReadStream *stream);
	void killEffects();
	void implement(Actor *enactor, GameObject *target, GameObject *strikingObj, uint8 strength);
};

//-----------------------------------------------------------------------


void initWeapons();
void cleanupWeapons();

WeaponStuff &getWeapon(weaponID i);

GameObject *getShieldItem(GameObject *defender);

} // End of namespace Saga2

#endif
