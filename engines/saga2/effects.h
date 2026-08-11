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
//   D Global Effects : Effects that have significant effect on the game engine
//

enum effectTypes {
	kEffectNone          = 0, // no functional effect
	kEffectAttrib,            // (enchant) affects attributes of actors
	kEffectResist,            // (enchant) Enable resistance to various things
	kEffectImmune,            // (enchant) Enable immunity to various things
	kEffectOthers,            // (enchant) Enable immunity to various things
	kEffectNonActor,          // (enchant) change an object
	kEffectPoison,            // (enchant) change an object
//  Effect types greater than 8 cannot be enchantments
	kEffectDamage        = 8,   // does damage of various types
	kEffectDrains,            // mana drain, money drain
	kEffectTAG,               // mana drain, money drain
	kEffectLocation,          // mana drain, money drain
	kEffectSpecial,
	kEffectStrike            // weapon strike effect
};


//
// Resistance Effects - these correspond exactly to the Damage types
//   A separate enum is defined to permit differentiation between
//   damage, resistance, and immunity effects
//

enum effectResistTypes {
	kResistOther     = kDamageOther,
	// Combat resist
	kResistImpact    = kDamageImpact,
	kResistSlash     = kDamageSlash,
	kResistProjectile = kDamageProjectile,
	// Magic resist
	kResistFire      = kDamageFire,
	kResistAcid      = kDamageAcid,
	kResistHeat      = kDamageHeat,
	kResistCold      = kDamageCold,
	kResistLightning = kDamageLightning,
	kResistPoison    = kDamagePoison,
	// Other magic resist
	kResistMental    = kDamageMental,
	kResistToUndead  = kDamageToUndead,
	kResistDirMagic  = kDamageDirMagic,
	// Physiological Damage
	kResistStarve    = kDamageStarve,
	// other
	kResistEnergy    = kDamageEnergy
};

//
// Immunity Effects - See the notes for resistance effects
//

// Types of damage an effect can give immunity to
enum effectImmuneTypes {
	kImmuneOther     = kResistOther,
	// Combat imm
	kImmuneImpact    = kResistImpact,
	kImmuneSlash     = kResistSlash,
	kImmuneProjectile = kResistProjectile,
	// Magic immu
	kImmuneFire      = kResistFire,
	kImmuneAcid      = kResistAcid,
	kImmuneHeat      = kResistHeat,
	kImmuneCold      = kResistCold,
	kImmuneLightning = kResistLightning,
	kImmunePoison    = kResistPoison,
	// Other magimune
	kImmuneMental    = kResistMental,
	kImmuneToUndead  = kResistToUndead,
	kImmuneDirMagic  = kResistDirMagic,
	// PhysiologiDamage
	kImmuneStarve    = kResistStarve,
	// other
	kImmuneEnergy    = kResistEnergy
};

//
// Other Effects - general flags in the actor structure most of which
//   aren't hooked up to anything.
//

enum effectOthersTypes {
	// Movement flags
	kActorNoncorporeal   = 1,    // The creature can walk through things
	kActorWaterBreathe   = 2,    // death spell
	kActorSlowFall       = 3,    // the creature is not harmed by falling (but falls none the less)
	kActorLevitate       = 4,    // flying with no height control ?
	kActorFlying         = 5,    // the creature flys
	// speed flags
	kActorFastMove       = 6,    //
	kActorFastAttack     = 7,    //
	kActorSlowAttack     = 8,    // come... back... here... lit... tle... bun... ny...

	kActorImmobile       = 9,    // I thought I told you to leave the piano at home
	// ill effects
	kActorAsleep         = 10,   // Zzzzzzzzzzz
	kActorParalyzed      = 11,   // the creature can't move an inch
	kActorFear           = 12,   // run away! run away
	kActorDiseased       = 13,   // cannot heal
	kActorPoisoned       = 14,   // death spell
	// perception & perceivability flags
	kActorBlind          = 15,   // can't see
	kActorSeeInvis       = 16,   // can see invisible
	kActorClairvoyant    = 17,  // unknown effects
	kActorInvisible      = 18,   // is invisible
	kActorUndetectable   = 19,   // can't be seen, smelled
	kActorDetPoison      = 20,   // poison things glow green
	// flags preventing changes to other flags
	kActorNoEnchant      = 21,   // no bad enchantments
	kActorNoDrain        = 22,   // no mana / food drains
	// flags that make things run away
	kActorRepelEvil      = 23,   // death spell
	kActorRepelGood      = 24,   // death spell
	kActorRepelUndead    = 25,   // death spell
	// miscellaneous
	kActorNotDefenseless = 26,   // forgo defenselessness check
	kActorDisappearOnDeath = 27, // gets deleted on death and spews inventory
	// dead or moved flags
	kActorWaterWalk           // can walk on water (same as float ?)
};

//
// Drains Effects - these correspond to values in the actor that are
//   drained & replenished
//

enum effectDrainsTypes {
	// mana pools
	kDrainsManaRed       = 1,
	kDrainsManaOrange,
	kDrainsManaYellow,
	kDrainsManaGreen,
	kDrainsManaBlue,
	kDrainsManaViolet,
	kDrainsLifeLevel,
	kDrainsVitality,
	kDrainsMoney
};


//
// TAG Effects - effects that apply when a TAG is the target
//

enum effectTAGTypes {
	kSettagLocked        = 1,
	kSettagOpen          = 2
};


//
// Location Effects - effects that apply when a Location is the target
//

enum kEffectLocationTypes {
	kLocateDummy         = 1
};

enum objectFlags {
	kObjectOpen          = (1 << 0),     // object is in the "open" state
	kObjectLocked        = (1 << 1),     // object cannot be opened
	kObjectImportant     = (1 << 2),     // object must be recycled when trashed
	kObjectGhosted       = (1 << 3),     // object drawn translucent
	kObjectInvisible     = (1 << 4),     // object cannot be seen
	kObjectObscured      = (1 << 5),     // object obscured by terrain
	kObjectMoving        = (1 << 6),     // object has attached motion task
	kObjectScavengable   = (1 << 7),     // object can be deleted
	kObjectFloating      = (1 << 8),     // object not affected by Gravity
	kObjectNoRecycle     = (1 << 9),     // object is referred to by script, don't delete
	kObjectActivated     = (1 << 10),    // object is activated
	kObjectAlias         = (1 << 11),    // object is not real, just a copy of another object
	kObjectTriggeringTAG = (1 << 12),    // object has triggerred TAG upon which it rests
	kObjectOnScreen      = (1 << 13),    // object is on display list
	kObjectSightedByCenter = (1 << 14)   // there is a line of sight to center actor
};


//
// Special Effects - these are spells that need to be handled manually
//

enum effectSpecialTypes {
	kSpecialDispellHelpfulEnch = 1,   // clears helpful enchantments
	kSpecialDispellHarmfulEnch,       // clears harmful enchantments
	kSpecialKill,                     // death spell
	kSpecialRessurect,                // raise dead spell
	kSpecialTeleport,                 // Teleportation
	kSpecialCreateActor,              // Create an actor or wall
	kSpecialSagaFunc,                 // calls a saga function
	kSpecialCreateWWisp,              // calls a saga function
	kSpecialCreateFWisp,              // calls a saga function
	kSpecialCreateWraith,             // calls a saga function
	kSpecialCreateFood,               // calls a saga function
	kSpecialRejoin
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
    {   return (kEffectAttrib << 13) | (atttyp << 8) + (damamt+128); }
*/

inline uint16 makeEnchantmentID(effectResistTypes restyp, bool damamt) {
	return ((kEffectResist << 13) | (restyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(effectImmuneTypes immtyp, bool damamt) {
	return ((kEffectImmune << 13) | (immtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(effectOthersTypes othtyp, bool damamt) {
	return ((kEffectOthers << 13) | (othtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(objectFlags othtyp, bool damamt) {
	return ((kEffectNonActor << 13) | (othtyp << 8)) + (damamt + 128);
}

inline uint16 makeEnchantmentID(uint8 damamt) {
	return ((kEffectPoison << 13) | (0 << 8)) + damamt;
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
	if (typ == kEffectAttrib)  return amt < 0;
	if (typ == kEffectOthers)
		return (sub >= kActorSlowAttack && sub <= kActorBlind);
	return false;
}

// ------------------------------------------------------------------
// Determine whether an enchantment can fail

inline bool isSaveable(uint16 enchID) {
	int16 typ = getEnchantmentType(enchID);
	return (typ == kEffectOthers && isHarmful(enchID));
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
	ProtoEffect *_next;                      // pointer to additional effects

	ProtoEffect() {
		_next = NULL;
	}
	virtual ~ProtoEffect() {
		if (_next) delete _next;
		_next = NULL;
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
	effectDamageTypes   _type;               // damage type
	int8                _dice,               // # of dice to roll
	                    _sides,              // # of sides on dice
	                    _skillDice,          // multiply by spellcraft to get additional dice
	                    _base,               // absolute damage amount
	                    _skillBase;          // absolute damage amount
	int8                _self;               // casts at self

public:

	ProtoDamage(int8 d, int8 s, int8 sd, int8 b, effectDamageTypes t, int, bool afSelf = false, int8 sb = 0) {
		_type = t;
		_dice = d;
		_sides = s;
		_skillDice = sd;
		_base = b;
		_self = afSelf;
		_skillBase = sb;
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
	effectDrainsTypes   _type;               // damage type
	int8                _dice,               // # of dice to roll
	                    _sides,              // # of sides on dice
	                    _skillDice,          // multiply by spellcraft to get additional dice
	                    _base;               // absolute damage amount
	int8                _self;               // casts at self

public:

	ProtoDrainage(int8 d, int8 s, int8 sd, int8 b, effectDrainsTypes t, int, bool afSelf = false) {
		_type = t;
		_dice = d;
		_sides = s;
		_skillDice = sd;
		_base = b;
		_self = afSelf;
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
	uint16              _enchID;
	uint32              _minEnch;
	RandomDice          _dice;               // enchantment time

public:
	ProtoEnchantment(uint16 e, uint32 loTime, uint32 hiTime) {
		_enchID = e;
		_dice = RandomDice(1, hiTime - loTime);
		_minEnch = loTime;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);

	bool canFail() {
		return isSaveable(_enchID);
	}

	static bool realSavingThrow(Actor *a);
};

//-------------------------------------------------------------------
// ProtoTAGEffect
//   this type of spell sets up spells that are used to alter tags

class ProtoTAGEffect: public ProtoEffect {
	effectTAGTypes      _affectBit;
	int16               _onOff;       // lock/unlock or trigger ID
	ObjectID            _trigger;

public:
	ProtoTAGEffect(effectTAGTypes ett, int16 v, ObjectID t) {
		_affectBit = ett;
		_onOff = v;
		_trigger = t;
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

//-------------------------------------------------------------------
// ProtoObjectEffect
//   These effects are used only on non-actor objects.

class ProtoObjectEffect: public ProtoEffect {
	uint16              _affectBit;
	int16               _onOff;
	RandomDice          _dice;               // enchantment time

public:
	ProtoObjectEffect(uint16 e, int16 v, uint32 loT, uint32 hiT) {
		_affectBit = e;
		_onOff = v;
		_dice = RandomDice(loT, hiT);
	}

	bool applicable(SpellTarget &trg);

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

//-------------------------------------------------------------------
// If spells ever need to do things to Locations this
// is where they'll be

class ProtoLocationEffect: public ProtoEffect {
	kEffectLocationTypes _affectBit;
	int16               _value;

public:
	ProtoLocationEffect(kEffectLocationTypes elt, int16 v) {
		_affectBit = elt;
		_value = v;
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
	int16 _routineID;
	SPELLIMPLEMENTATION *_handler;

public:
	ProtoSpecialEffect(SPELLIMPLEMENTATION *newHandler, int16 callID = 0) {
		_handler = newHandler;
		_routineID = callID;
	}

	bool applicable(SpellTarget &) {
		return true;
		//return (trg.getType()==SpellTarget::kSpellTargetObject ||
		//       trg.getType()==SpellTarget::kSpellTargetObjectPoint) &&
		//     isActor(trg.getObject());
	}

	void implement(GameObject *, SpellTarget *trg, int8 deltaDamage = 0);
};

} // end of namespace Saga2

#endif
