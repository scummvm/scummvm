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

#ifndef SAGA2_EFFECTS_H
#define SAGA2_EFFECTS_H

#include "saga2/dice.h"

namespace Saga2 {

class Actor;
class GameObject;

// ------------------------------------------------------------------
// Effects of spells and other things

//
// 1 Enchantments
//   A Object (non-Actor) Enchantments
//       Object : There aren't a lot of these
//   B Actor Enchantments
//       Attrib : affects attributes of actors
//       Resist : Enable resistance to various things
//       Immune : Enable immunity to various things
//       Others : Misc flags
//	 C Player Enchantments
//       Player : Flags that really only affect players
// 2 Effects
//   A General effects
//       Damage : does damage of various types
//   B Actor only effects
//       Drains : mana drains, money drains, food drains
//       Special : must be handled manually
//   C TAG effects : There aren't many of these
//   D Global Effects : Effects that have signifigant effect on the game engine
//

enum effectTypes {
	effectNone          = 0, // no functional effect
	effectAttrib,            // (enchant) affects attributes of actors
	effectResist,            // (enchant) Enable resistance to various things
	effectImmune,            // (enchant) Enable immunity to various things
	effectOthers,            // (enchant) Enable immunity to various things
	effectNonActor,          // (enchant) change an object
	effectPoison,            // (enchant) change an object
//  Effect types greater than 8 cannot be enchantments
	effectDamage        = 8,   // does damage of various types
	effectDrains,            // mana drain, money drain
	effectTAG,               // mana drain, money drain
	effectLocation,          // mana drain, money drain
	effectSpecial,
	effectStrike            // weapon strike effect
};


//
// Resistance Effects - these correspond exactly to the Damage types
//   A separate enum is defined to permit differentiation between
//   damage, resistance, and immunity effects
//

enum effectResistTypes {
	resistOther     = kDamageOther,
	// Combat resist
	resistImpact    = kDamageImpact,
	resistSlash     = kDamageSlash,
	resistProjectile = kDamageProjectile,
	// Magic resist
	resistFire      = kDamageFire,
	resistAcid      = kDamageAcid,
	resistHeat      = kDamageHeat,
	resistCold      = kDamageCold,
	resistLightning = kDamageLightning,
	resistPoison    = kDamagePoison,
	// Other magic resist
	resistMental    = kDamageMental,
	resistToUndead  = kDamageToUndead,
	resistDirMagic  = kDamageDirMagic,
	// Physiological Damage
	resistStarve    = kDamageStarve,
	// other
	resistEnergy    = kDamageEnergy
};

//
// Immunity Effects - See the notes for resistance effects
//

// Types of damage an effect can give immunity to
enum effectImmuneTypes {
	immuneOther     = resistOther,
	// Combat imm
	immuneImpact    = resistImpact,
	immuneSlash     = resistSlash,
	immuneProjectile = resistProjectile,
	// Magic immu
	immuneFire      = resistFire,
	immuneAcid      = resistAcid,
	immuneHeat      = resistHeat,
	immuneCold      = resistCold,
	immuneLightning = resistLightning,
	immunePoison    = resistPoison,
	// Other magimune
	immuneMental    = resistMental,
	immuneToUndead  = resistToUndead,
	immuneDirMagic  = resistDirMagic,
	// PhysiologiDamage
	immuneStarve    = resistStarve,
	// other
	immuneEnergy    = resistEnergy
};

//
// Other Effects - general flags in the actor structure most of which
//   aren't hooked up to anything.
//

enum effectOthersTypes {
	// Movement flags
	actorNoncorporeal   = 1,    // The creature can walk through things
	actorWaterBreathe   = 2,    // death spell
	actorSlowFall       = 3,    // the creature is not harmed by falling (but falls none the less)
	actorLevitate       = 4,    // flying with no height control ?
	actorFlying         = 5,    // the creature flys
	// speed flags
	actorFastMove       = 6,    //
	actorFastAttack     = 7,    //
	actorSlowAttack     = 8,    // come... back... here... lit... tle... bun... ny...

	actorImmobile       = 9,    // I thought I told you to leave the piano at home
	// ill effects
	actorAsleep         = 10,   // Zzzzzzzzzzz
	actorParalyzed      = 11,   // the creature can't move an inch
	actorFear           = 12,   // run away! run away
	actorDiseased       = 13,   // cannot heal
	actorPoisoned       = 14,   // death spell
	// perception & perceivability flags
	actorBlind          = 15,   // can't see
	actorSeeInvis       = 16,   // can see invisible
	actorClairvoyant    = 17,  // unknown effects
	actorInvisible      = 18,   // is invisible
	actorUndetectable   = 19,   // can't be seen, smelled
	actorDetPoison      = 20,   // poison things glow green
	// flags preventing changes to other flags
	actorNoEnchant      = 21,   // no bad enchantments
	actorNoDrain        = 22,   // no mana / food drains
	// flags that make things run away
	actorRepelEvil      = 23,   // death spell
	actorRepelGood      = 24,   // death spell
	actorRepelUndead    = 25,   // death spell
	// miscellaneous
	actorNotDefenseless = 26,   // forgo defenselessness check
	actorDisappearOnDeath = 27, // gets deleted on death and spews inventory
	// dead or moved flags
	actorWaterWalk           // can walk on water (same as float ?)
};

//
// Drains Effects - these correspond to values in the actor that are
//   drained & replenished
//

enum effectDrainsTypes {
	// mana pools
	drainsManaRed       = 1,
	drainsManaOrange,
	drainsManaYellow,
	drainsManaGreen,
	drainsManaBlue,
	drainsManaViolet,
	drainsLifeLevel,
	drainsVitality,
	drainsMoney
};


//
// TAG Effects - effects that apply when a TAG is the target
//

enum effectTAGTypes {
	settagLocked        = 1,
	settagOpen          = 2
};


//
// Location Effects - effects that apply when a Location is the target
//

enum effectLocationTypes {
	locateDummy         = 1
};

enum objectFlags {
	objectOpen          = (1 << 0),     // object is in the "open" state
	objectLocked        = (1 << 1),     // object cannot be opened
	objectImportant     = (1 << 2),     // object must be recycled when trashed
	objectGhosted       = (1 << 3),     // object drawn translucent
	objectInvisible     = (1 << 4),     // object cannot be seen
	objectObscured      = (1 << 5),     // object obscured by terrain
	objectMoving        = (1 << 6),     // object has attached motion task
	objectScavengable   = (1 << 7),     // object can be deleted
	objectFloating      = (1 << 8),     // object not affected by Gravity
	objectNoRecycle     = (1 << 9),     // object is referred to by script, don't delete
	objectActivated     = (1 << 10),    // object is activated
	objectAlias         = (1 << 11),    // object is not real, just a copy of another object
	objectTriggeringTAG = (1 << 12),    // object has triggerred TAG upon which it rests
	objectOnScreen      = (1 << 13),    // object is on display list
	objectSightedByCenter = (1 << 14)   // there is a line of sight to center actor
};


//
// Special Effects - these are spells that need to be handled manually
//

enum effectSpecialTypes {
	specialDispellHelpfulEnch = 1,   // clears helpful enchantments
	specialDispellHarmfulEnch,       // clears harmful enchantments
	specialKill,                     // death spell
	specialRessurect,                // raise dead spell
	specialTeleport,                 // Teleportation
	specialCreateActor,              // Create an actor or wall
	specialSagaFunc,                 // calls a saga function
	specialCreateWWisp,              // calls a saga function
	specialCreateFWisp,              // calls a saga function
	specialCreateWraith,             // calls a saga function
	specialCreateFood,               // calls a saga function
	specialRejoin
};

// ------------------------------------------------------------------
// ENCHANTMENT IDs
// It is necessary to combine all these possibilities into a 16 bit integer
// Here's how its mapped
//   3 bits - general effect type
//   5 bits - sub class enum value
//   8 bits - damage amount, boolean on/off etc.
//

inline uint16 makeEnchantmentID(uint16 type, uint16 damtyp, int16 damamt) {
	assert(type < 8);
	assert(damtyp < 32);
	assert(damamt < 128 && damamt > -128);
	return ((type << 13) | (damtyp << 8)) + (damamt + 128);
}

/* skill*  are now in the spellid enum ;AS;
inline uint16 makeEnchantmentID(effectAttribTypes atttyp, int16 damamt)
    {   return (effectAttrib << 13) | (atttyp << 8) + (damamt+128); }
*/

inline uint16 makeEnchantmentID(effectResistTypes restyp, bool damamt) {
	return ((effectResist << 13) | (restyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(effectImmuneTypes immtyp, bool damamt) {
	return ((effectImmune << 13) | (immtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(effectOthersTypes othtyp, bool damamt) {
	return ((effectOthers << 13) | (othtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(objectFlags othtyp, bool damamt) {
	return ((effectNonActor << 13) | (othtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(uint8 damamt) {
	return ((effectPoison << 13) | (0 << 8)) + damamt;
}

inline effectTypes getEnchantmentType(uint16 eID) {
	return (effectTypes)(eID >> 13);
}

inline uint16 getEnchantmentSubType(uint16 eID) {
	return (eID >> 8) & 0x1F;
}

inline int16 getEnchantmentAmount(uint16 eID) {
	return (eID & 0xFF) - 128;
}

// ------------------------------------------------------------------
// Determine whether an enchantment is harmful

inline bool isHarmful(uint16 enchID) {
	int16 typ = getEnchantmentType(enchID);
	int16 sub = getEnchantmentSubType(enchID);
	int16 amt = getEnchantmentAmount(enchID);
	if (typ == effectAttrib)  return amt < 0;
	if (typ == effectOthers)
		return (sub >= actorSlowAttack && sub <= actorBlind);
	return false;
}

// ------------------------------------------------------------------
// Determine whether an enchantment can fail

inline bool isSaveable(uint16 enchID) {
	int16 typ = getEnchantmentType(enchID);
	return (typ == effectOthers && isHarmful(enchID));
}

// ------------------------------------------------------------------
//  Determine whether a damage type is magical

inline bool isMagicDamage(effectDamageTypes t) {
	return t >= kDamageFire && t <= kDamageDirMagic;
}

#define Forever (255)

class SpellTarget;

//-------------------------------------------------------------------
// ProtoEffects

//   This is the base class of several spell effect prototype classes
//     The implement routine carries out the instantiation of a
//     particular effect on a given target (doing damage or whatever)


class ProtoEffect {
	//protected:
	//int imp;                // enchant or immediate

public:
	ProtoEffect *next;                      // pointer to additional effects

	ProtoEffect() {
		next = NULL;
	}
	virtual ~ProtoEffect() {
		if (next) delete next;
		next = NULL;
	}
	//int implementation( void ) { return imp; }
	virtual bool applicable(SpellTarget &) {
		return false;
	}
	virtual void implement(GameObject *, SpellTarget *, int8 = 0) {}
};


//-------------------------------------------------------------------
// ProtoDamage
//   This class of effects does a range of damage to the target

class ProtoDamage: public ProtoEffect {
	effectDamageTypes   type;               // damage type
	int8                dice,               // # of dice to roll
	                    sides,              // # of sides on dice
	                    skillDice,          // multiply by spellcraft to get additional dice
	                    base,               // absolute damage amount
	                    skillBase;               // absolute damage amount
	int8                self;               // casts at self

public:

	ProtoDamage(int8 d, int8 s, int8 sd, int8 b, effectDamageTypes t, int, bool afSelf = false, int8 sb = 0) {
		type = t;
		dice = d;
		sides = s;
		skillDice = sd;
		base = b;
		self = afSelf;
		skillBase = sb;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *cst, SpellTarget *trg, int8 deltaDamage = 0);

	static int16 getRelevantStat(effectDamageTypes dt, Actor *a);
};

//-------------------------------------------------------------------
// ProtoDrainage
//   This class of effects does a range of damage to the target's
//   mana, money or food supply

class ProtoDrainage: public ProtoEffect {
	effectDrainsTypes   type;               // damage type
	int8                dice,               // # of dice to roll
	                    sides,              // # of sides on dice
	                    skillDice,          // multiply by spellcraft to get additional dice
	                    base;               // absolute damage amount
	int8                self;               // casts at self

public:

	ProtoDrainage(int8 d, int8 s, int8 sd, int8 b, effectDrainsTypes t, int, bool afSelf = false) {
		type = t;
		dice = d;
		sides = s;
		skillDice = sd;
		base = b;
		self = afSelf;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *cst, SpellTarget *trg, int8 deltaDamage = 0);

	static int16 currentLevel(Actor *a, effectDrainsTypes edt);
	static void drainLevel(GameObject *cst, Actor *a, effectDrainsTypes edt, int16 amt);
};

//-------------------------------------------------------------------
// ProtoEnchantment
//   This can be any of several types of enchantments (see EFFECTS.H)
//

class ProtoEnchantment: public ProtoEffect {
	uint16              enchID;
	uint32              minEnch;
	RandomDice          dice;               // enchantment time

public:
	ProtoEnchantment(uint16 e, uint32 loTime, uint32 hiTime) {
		enchID = e;
		dice = RandomDice(1, hiTime - loTime);
		minEnch = loTime;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);

	bool canFail() {
		return isSaveable(enchID);
	}

	static bool realSavingThrow(Actor *a);
};

//-------------------------------------------------------------------
// ProtoTAGEffect
//   this type of spell sets up spells that are used to alter tags

class ProtoTAGEffect: public ProtoEffect {
	effectTAGTypes      affectBit;
	int16               onOff;       // lock/unlock or trigger ID
	ObjectID            trigger;

public:
	ProtoTAGEffect(effectTAGTypes ett, int16 v, ObjectID t) {
		affectBit = ett;
		onOff = v;
		trigger = t;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

//-------------------------------------------------------------------
// ProtoObjectEffect
//   These effects are used only on non-actor objects.

class ProtoObjectEffect: public ProtoEffect {
	uint16              affectBit;
	int16               onOff;
	RandomDice          dice;               // enchantment time

public:
	ProtoObjectEffect(uint16 e, int16 v, uint32 loT, uint32 hiT) {
		affectBit = e;
		onOff = v;
		dice = RandomDice(loT, hiT);
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

//-------------------------------------------------------------------
// If spells ever need to do things to Locations this
// is where they'll be

class ProtoLocationEffect: public ProtoEffect {
	effectLocationTypes affectBit;
	int16               value;

public:
	ProtoLocationEffect(effectLocationTypes elt, int16 v) {
		affectBit = elt;
		value = v;
	}

	bool applicable(SpellTarget &)  {
		return (true);
	}

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

//-------------------------------------------------------------------
// ProtoSpecialEffects
//   As always there are spells that just don't fit any of the other
//   molds. These protoEffects allow customized spell handlers to be
//   implemented.
//

typedef void SPELLIMPLEMENTATION(GameObject *, SpellTarget *);

#define SPECIALSPELL(name) void name(GameObject *cst, SpellTarget *trg)

class ProtoSpecialEffect: public ProtoEffect {
	int16 routineID;
	SPELLIMPLEMENTATION *handler;

public:
	ProtoSpecialEffect(SPELLIMPLEMENTATION *newHandler, int16 callID = 0) {
		handler = newHandler;
		routineID = callID;
	}

	bool applicable(SpellTarget &) {
		return true;
		//return (trg.getType()==SpellTarget::spellTargetObject ||
		//       trg.getType()==SpellTarget::spellTargetObjectPoint) &&
		//     isActor(trg.getObject());
	}

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

} // end of namespace Saga2

#endif
