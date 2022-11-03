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

#ifndef SAGA2_SPELLBUK_H
#define SAGA2_SPELLBUK_H

namespace Saga2 {

struct ResourceSpellEffect;
struct ResourceSpellItem;

class SpellTarget;
class ProtoEffect;

// Mana IDs as spells see them

enum SpellManaID {
	ksManaIDRed      = 0,
	ksManaIDOrange   = 1,
	ksManaIDYellow   = 2,
	ksManaIDGreen    = 3,
	ksManaIDBlue     = 4,
	ksManaIDViolet   = 5,
	ksManaIDSkill    = 6         // skills are here for convenience
};

//-------------------------------------------------------------------
// targeting bits
//   These two types are used to determine :
//     The types of screen object a spell can be used on
//     The type of target the spell will eventually be applied to
//   respectively.

//-------------------------------------------------------------------
// legal target selections
enum SpellTargetingTypes {
	kSpellTargNone       = 0,
	kSpellTargWorld      = 1 << 0, // instant spell
	kSpellTargLocation   = 1 << 1, // cast at any location on map
	kSpellTargTAG        = 1 << 2, // cast at tileactivity inst.
	kSpellTargObject     = 1 << 3, // cast at objects
	kSpellTargActor      = 1 << 4,
	kSpellTargCaster     = 1 << 5
};

//-------------------------------------------------------------------
// target type the spell uses when implemented
enum SpellApplicationTypes {
	kSpellApplyNone      = kSpellTargNone,
	kSpellApplyWorld     = kSpellTargWorld,
	kSpellApplyLocation  = kSpellTargLocation,
	kSpellApplyTAG       = kSpellTargTAG,
	kSpellApplyObject    = kSpellTargObject,
	kSpellApplyActor     = kSpellTargObject,
	kSpellApplyTracking  = 1 << 6  // track object targets
};


//-------------------------------------------------------------------
// effect templates
//   These are the shapes of the visible effects of spells

enum effectAreas {
	keAreaInvisible = 0,
	keAreaAura,
	keAreaProjectile,
	keAreaExchange,
	keAreaBolt,
	keAreaCone,
	keAreaBall,
	keAreaSquare,
	keAreaWave,
	keAreaStorm,
	keAreaMissle,
	keAreaGlow,
	keAreaBeam,
	keAreaWall
};



//-------------------------------------------------------------------
// SpellStuff
//   The master spell list is an array of these records.
//   Unfortunately this class and the SpellDisplayProto could have been
//   implemented as one larger structure, but the evolved from separate
//   parts of the code

class SpellStuff {
	SpellID             _master;             // index in array
	SkillProto          *_prototype;         // ponts back to object prototype
	SpellID             _display;            // currently same as master
	SpellTargetingTypes _targetableTypes;    // valid targeting types
	SpellApplicationTypes _targetTypes;      // the targeting type to implement
	ProtoEffect         *_effects;           // the effects of this spell
	SpellTarget         *_targets;           // transient target list
	SpellManaID         _manaType;           // color mana used
	int8                _manaUse;            // mana points used
	effectAreas         _shape;
	int32               _size;
	int32               _range;
	int16               _sound;

	bool _debug;

public:

	SpellStuff();

	void setProto(SkillProto *p) {
		_prototype = p;
	}
	SkillProto *getProto() {
		return _prototype;
	}

	void setupFromResource(ResourceSpellItem *rsi);

	void addEffect(ProtoEffect *pe);
	void addEffect(ResourceSpellEffect *rse);
	void killEffects();

	bool canTarget(SpellTargetingTypes t) {
		return _targetableTypes & t;
	}
	bool shouldTarget(SpellApplicationTypes t) {
		return _targetTypes & t;
	}

	bool untargetable() {
		return (_targetableTypes == kSpellTargNone);
	}
	bool untargeted() {
		return false;    //(targetableTypes == kSpellTargWorld ) ||
	}
	//(targetableTypes == kSpellTargCaster ) ||
	//(targetableTypes == targetableTypes &
	//                   (kSpellTargWorld | kSpellTargCaster)); }

	void implement(GameObject *enactor, SpellTarget *target);
	void implement(GameObject *enactor, GameObject *target);
	void implement(GameObject *enactor, ActiveItem *target);
	void implement(GameObject *enactor, Location   target);

	SpellID getDisplayID() {
		return _display;
	}
	SpellManaID getManaType() {
		return _manaType;
	}
	void setManaType(SpellManaID smid) {
		_manaType = smid;
	}
	int8 getManaAmt() {
		return _manaUse;
	}
	int32 getRange() {
		return _range;
	}

	void buildTargetList(GameObject *, SpellTarget &);
	void addTarget(SpellTarget *trg);
	void removeTargetList();

	void apply(ProtoEffect *pe, GameObject *target);
	void apply(ProtoEffect *pe, ActiveItem *target);

	void playSound(GameObject *go);
	void show(GameObject *, SpellTarget &);
	bool safe();
	bool isOffensive();
};

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

//-------------------------------------------------------------------
// At this point these are the effects requiring special handling

SPECIALSPELL(DeathSpell);
SPECIALSPELL(DispellProtections);
SPECIALSPELL(DispellCurses);
SPECIALSPELL(Resurrect);
SPECIALSPELL(CreateWallOfFire);
SPECIALSPELL(CreateFireWisp);
SPECIALSPELL(CreateWindWisp);
SPECIALSPELL(Timequake);
SPECIALSPELL(TeleportToShrine);
SPECIALSPELL(TeleportToLocation);
SPECIALSPELL(Rejoin);
SPECIALSPELL(CreateRingOfForce);
SPECIALSPELL(DispellPoison);
SPECIALSPELL(CreateWraith);
SPECIALSPELL(SagaSpellCall);
SPECIALSPELL(CreateWWisp);
SPECIALSPELL(CreateFWisp);
SPECIALSPELL(CreateFood);

} // End of namespace Saga2

#endif  //SPELLBUK_H
