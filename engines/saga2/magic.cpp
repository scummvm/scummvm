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
#include "saga2/magic.h"
#include "saga2/idtypes.h"
#include "saga2/speldefs.h"
#include "saga2/spellbuk.h"
#include "saga2/spelshow.h"
#include "saga2/motion.h"
#include "saga2/player.h"

namespace Saga2 {

//-----------------------------------------------------------------------
// ASSUMPTIONS
//   Wands and the like will use only 1 charge per use
//   The enactor will either be an object in the world or in the
//     inventory of an actor in the world
//   Spell objects will have IDs > the number of spells
//

/* ===================================================================== *
   Constants
 * ===================================================================== */

#define RANGE_CHECKING 0
#define NPC_MANA_CHECK 0

const int32 spellFailSound = 42;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern WorldMapData                     *mapList;
extern SpellStuff                       *spellBook;
extern Point32                          lastMousePos;           // Last mouse position over map

/* ===================================================================== *
   Inlines
 * ===================================================================== */

//-----------------------------------------------------------------------
// Notify target of ill intent
void logAggressiveAct(ObjectID attackerID, ObjectID attackeeID);

//-----------------------------------------------------------------------
// This hack is here so wands & such will be implemented as if
//   the owner had cast the spell while the mana comes from the object
GameObject *GetOwner(GameObject *go) {
	GameObject      *obj = go;
	ObjectID        id;
	for (;;) {
		id = obj->parent()->thisID();
		if (isWorld(id))
			return obj;
		else if (id == Nothing)
			return go;

		obj = GameObject::objectAddress(id);
	}
}


/* ===================================================================== *
   Magic code
 * ===================================================================== */

//-----------------------------------------------------------------------
// This call looks up a spells object prototype. It can accept either
//   an object ID or a spell ID
SkillProto *skillProtoFromID(int16 spellOrObjectID) {
	if (spellOrObjectID > MAX_SPELLS)
		return (SkillProto *)GameObject::protoAddress(spellOrObjectID);

	if (spellOrObjectID >= totalSpellBookPages)
		error("Wrong spellID: %d > %d", spellOrObjectID, totalSpellBookPages);

	return spellBook[spellOrObjectID].getProto();
}

//-----------------------------------------------------------------------
// initialization call to connect skill prototypes with their spells
void initializeSkill(SkillProto *oNo, SpellID sNo) {
	if (sNo > 0 && sNo < totalSpellBookPages) {
		if (spellBook[sNo].getProto() != nullptr)
			error("Duplicate prototype for spell %d", sNo);
		spellBook[sNo].setProto(oNo);
	} else
		warning("Spell prototype has invalid spell ID %d (lockType)", sNo);
}

//-----------------------------------------------------------------------
// test for untargeted skill/spell which gets cast immediately
bool nonTargeted(SkillProto *spell) {
	SpellID    sid = spell->getSpellID();
	return spellBook[sid].untargeted();
}

//-----------------------------------------------------------------------
// test for untargeted skill/spell which gets cast immediately
bool nonUsable(SkillProto *spell) {
	SpellID    sid = spell->getSpellID();
	return spellBook[sid].untargetable();
}

//-----------------------------------------------------------------------
// test a target for viability in a given spell
bool validTarget(GameObject *enactor, GameObject *target, ActiveItem *tag, SkillProto *skill) {
	assert(enactor != nullptr);
	assert(skill != nullptr);
	SpellStuff &sp = spellBook[skill->getSpellID()];
	int32 range = sp.getRange();
	if (target != nullptr && target->thisID() != Nothing) {
#if RANGE_CHECKING
		if (range > 0 &&
		        range > (
		            enactor->getLocation() -
		            target->getLocation())
		        .magnitude()) {
			return false;
		}
#endif
		if (target->IDParent() != enactor->IDParent()) {
			return false;
		}
		if (!lineOfSight(enactor, target, terrainTransparent))
			return false;

		if (isActor(target)) {
			Actor *a = (Actor *) target;
			Actor *e = (Actor *) enactor;
			if (a->hasEffect(actorInvisible) && !e->hasEffect(actorSeeInvis))
				return false;
		}
		if (target->thisID() == enactor->thisID())
			return sp.canTarget(spellTargCaster);
		return sp.canTarget(spellTargObject);
	}
	if (tag != nullptr) {
		if (range > 0 &&
		        range > (
		            enactor->getWorldLocation() -
		            TAGPos(tag))
		        .magnitude()) {
			return false;
		}
		return sp.canTarget(spellTargTAG);
	}
#if RANGE_CHECKING
	if (sp.range > 0 &&
	        sp.range > (
	            enactor->getLocation() -
	        )
	        .magnitude()) {
		return false;
	}
#endif
	return sp.canTarget(spellTargLocation);
}

//-----------------------------------------------------------------------
// check for sufficient mana
bool canCast(GameObject *enactor, SkillProto *spell) {
	SpellID s = spell->getSpellID();
	SpellStuff &sProto = spellBook[s];
	ActorManaID ami = (ActorManaID)(sProto.getManaType());
	int amt = sProto.getManaAmt();

	if (ami == numManas)
		return true;
#if NPC_MANA_CHECK
	if (isActor(enactor)) {
		Actor *a = (Actor *) enactor;
		assert(ami >= manaIDRed && ami <= manaIDViolet);
		if ((&a->effectiveStats.redMana)[ami] < amt)
			return false;
		return true;
	} else {
		return true;
	}
#endif
	return enactor->hasCharge(ami, amt);

}

/* ===================================================================== *
   Spell casting code
 * ===================================================================== */

//  There are two levels of routines here:
//    Cast: This call makes the actor do his spell casting dance. The
//      motion task calls the implement routine at the appropriate
//      time. If the caster is not an actor the implement routine is
//      chained directly.
//    Implement: This call actually causes the spell to 'fire'. The
//      display effect is started, and the internal effects are
//      implemented.
//

//-----------------------------------------------------------------------
// cast untargeted spell
bool castUntargetedSpell(GameObject *enactor, SkillProto *spell) {
	castSpell(enactor, enactor, spell);
	return true;
}

//-----------------------------------------------------------------------
// cast a spell at a location
bool castSpell(GameObject *enactor, Location   &target, SkillProto *spell) {
	if (enactor) {
		if (isActor(enactor)) {
			Actor *a = (Actor *) enactor;
			MotionTask::castSpell(*a, *spell, target);
		} else
			implementSpell(enactor, target, spell);
	}
	return true;
}

//-----------------------------------------------------------------------
// cast a spell at a TAG
bool castSpell(GameObject *enactor, ActiveItem *target, SkillProto *spell) {
	if (enactor && target) {
		if (isActor(enactor)) {
			Actor *a = (Actor *) enactor;
			MotionTask::castSpell(*a, *spell, *target);
		} else
			implementSpell(enactor, target, spell);
	}
	return true;
}

//-----------------------------------------------------------------------
// cast a spell at an object
bool castSpell(GameObject *enactor, GameObject *target, SkillProto *spell) {
	SpellID s = spell->getSpellID();
	SpellStuff &sProto = spellBook[s];
	if (sProto.isOffensive())
		logAggressiveAct(enactor->thisID(), target->thisID());

	if (enactor && target) {
		if (isActor(enactor)) {
			Actor *a = (Actor *) enactor;
			MotionTask::castSpell(*a, *spell, *target);
		} else
			implementSpell(enactor, target, spell);
	}
	return true;
}

//-----------------------------------------------------------------------
// implement a spell at a location
bool implementSpell(GameObject *enactor, Location   &target, SkillProto *spell) {
	SpellID s = spell->getSpellID();
	SpellStuff &sProto = spellBook[s];

	assert(sProto.shouldTarget(spellApplyLocation));

	ActorManaID ami = (ActorManaID)(sProto.getManaType());

	if (isActor(enactor)) {
		Actor *a = (Actor *) enactor;
		bool r = a->takeMana(ami, sProto.getManaAmt());
		if (!r) {
			Location cal = Location(a->getLocation(), a->IDParent());
			Saga2::playSoundAt(MKTAG('S', 'P', 'L', spellFailSound), cal);
			return false;
		}
		PlayerActorID       playerID;

		if (actorIDToPlayerID(enactor->thisID(), playerID)) {
			PlayerActor     *player = getPlayerActorAddress(playerID);

			player->skillAdvance(skillIDSpellcraft, sProto.getManaAmt() / 10);
		}
	} else {
		if (!enactor->deductCharge(ami, sProto.getManaAmt())) {
			return false;
		}
	}

	g_vm->_activeSpells->add(new SpellInstance(GetOwner(enactor), target, sProto.getDisplayID()));
	sProto.playSound(enactor);
	return true;
}

//-----------------------------------------------------------------------
// implement a spell at a TAG
bool implementSpell(GameObject *enactor, ActiveItem *target, SkillProto *spell) {
	SpellID s = spell->getSpellID();
	SpellStuff &sProto = spellBook[s];
	Location l = Location(TAGPos(target), enactor->world()->thisID());
	if (sProto.shouldTarget(spellApplyLocation)) {
		return implementSpell(enactor, l, spell);
	}
	assert(sProto.shouldTarget(spellApplyTAG));
	assert(target->_data.itemType == activeTypeInstance);

	ActorManaID ami = (ActorManaID)(sProto.getManaType());

	if (isActor(enactor)) {
		Actor *a = (Actor *) enactor;
		bool r = a->takeMana(ami, sProto.getManaAmt());
		if (!r) {
			Location cal = Location(a->getLocation(), a->IDParent());
			Saga2::playSoundAt(MKTAG('S', 'P', 'L', spellFailSound), cal);
			return false;
		}
		PlayerActorID       playerID;

		if (actorIDToPlayerID(enactor->thisID(), playerID)) {
			PlayerActor     *player = getPlayerActorAddress(playerID);

			player->skillAdvance(skillIDSpellcraft, sProto.getManaAmt() / 10);
		}
	} else {
		if (!enactor->deductCharge(ami, sProto.getManaAmt())) {
			return false;
		}
	}

	g_vm->_activeSpells->add(new SpellInstance(GetOwner(enactor), l, sProto.getDisplayID()));
	sProto.playSound(enactor);
	return true;
}

//-----------------------------------------------------------------------
// implement a spell at an object
bool implementSpell(GameObject *enactor, GameObject *target, SkillProto *spell) {
	SpellID s = spell->getSpellID();
	SpellStuff &sProto = spellBook[s];
	Location l = Location(target->getWorldLocation(), enactor->world()->thisID());
	if (sProto.shouldTarget(spellApplyLocation))
		return implementSpell(enactor, l, spell);
	assert(sProto.shouldTarget(spellApplyObject));

	ActorManaID ami = (ActorManaID)(sProto.getManaType());

	if (isActor(enactor)) {
		Actor *a = (Actor *) enactor;
		bool r = a->takeMana(ami, sProto.getManaAmt());
		if (!r) {
			Location cal = Location(a->getLocation(), a->IDParent());
			Saga2::playSoundAt(MKTAG('S', 'P', 'L', spellFailSound), cal);
			return false;
		}
		PlayerActorID       playerID;

		if (actorIDToPlayerID(enactor->thisID(), playerID)) {
			PlayerActor     *player = getPlayerActorAddress(playerID);

			player->skillAdvance(skillIDSpellcraft, sProto.getManaAmt() / 10);
		}
	} else {
		if (!enactor->deductCharge(ami, sProto.getManaAmt())) {
			return false;
		}
	}

	g_vm->_activeSpells->add(new SpellInstance(GetOwner(enactor), target, sProto.getDisplayID()));
	sProto.playSound(enactor);
	return true;
}

} // end of namespace Saga2
