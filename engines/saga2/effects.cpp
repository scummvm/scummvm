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
#include "saga2/cmisc.h"
#include "saga2/spelshow.h"
#include "saga2/script.h"
#include "saga2/actor.h"

namespace Saga2 {

const int16 absoluteMaximumVitality = 255;

extern void updateIndicators();      //  Kludge, put in intrface.h later (got to hurry)

// offensiveNotification gets 2 (Actor *) items
// att is performing an offensive act on def
#define offensiveNotification(att,def) ((def)->handleOffensiveAct(att))

/* ===================================================================== *
   Effect Implementations
 * ===================================================================== */

// ------------------------------------------------------------------
// Determine stat that modifies damage

int16 ProtoDamage::getRelevantStat(effectDamageTypes dt, Actor *a) {
	switch (dt) {

	case kDamageImpact     :
	case kDamageSlash      :
	case kDamageProjectile :
		return a->getStats()->getSkillLevel(skillIDBrawn);
	case kDamageFire       :
	case kDamageAcid       :
	case kDamageHeat       :
	case kDamageCold       :
	case kDamageLightning  :
	case kDamagePoison     :
	case kDamageMental     :
	case kDamageToUndead   :
		return a->getStats()->getSkillLevel(skillIDSpellcraft);
	case kDamageDirMagic   :
	case kDamageOther      :
	case kDamageStarve     :
	case kDamageEnergy     :
		return 0;
	}
	return 0;
}

// ------------------------------------------------------------------
// Cause damage to something based on a damage proto-effect

void ProtoDamage::implement(GameObject *cst, SpellTarget *trg, int8 deltaDamage) {
	int8 totalDice;
	int8 totalBase;
	if (isActor(cst)) {
		Actor *a = (Actor *) cst;
		totalDice = dice + skillDice * getRelevantStat(type, a);
		totalBase = base + skillBase * getRelevantStat(type, a);
		if (totalDice > 0 && trg->getObject() && isActor(trg->getObject()))
			offensiveNotification(a, (Actor *) trg->getObject());
	} else {
		totalDice = dice;
		totalBase = base;
		ObjectID pID = cst->possessor();
		if (pID != Nothing) {
			Actor *p = (Actor *) GameObject::objectAddress(pID);
			assert(isActor(p));
			if (totalDice > 0 && trg->getObject() && isActor(trg->getObject()))
				offensiveNotification(p, (Actor *) trg->getObject());
		}
	}

	totalBase -= deltaDamage;

	assert(trg->getType() == SpellTarget::spellTargetObject);
	if (self)
		cst->acceptDamage(cst->thisID(), totalBase, type, totalDice, sides);
	else
		trg->getObject()->acceptDamage(cst->thisID(), totalBase, type, totalDice, sides);
}

// ------------------------------------------------------------------
// drain something based on a drainage proto-effect

int16 ProtoDrainage::currentLevel(Actor *a, effectDrainsTypes edt) {
	switch (edt) {
	case drainsManaRed:
	case drainsManaOrange:
	case drainsManaYellow:
	case drainsManaGreen:
	case drainsManaBlue:
	case drainsManaViolet:
		return (&a->_effectiveStats.redMana)[edt - drainsManaRed];

	case drainsLifeLevel:
		return (a->getBaseStats())->vitality;
	case drainsVitality:
		return a->_effectiveStats.vitality;
	default:
		return 0;
	}
}

void ProtoDrainage::drainLevel(GameObject *cst, Actor *a, effectDrainsTypes edt, int16 amt) {
	switch (edt) {
	case drainsManaRed:
	case drainsManaOrange:
	case drainsManaYellow:
	case drainsManaGreen:
	case drainsManaBlue:
	case drainsManaViolet:
		{
			ActorManaID aType = (ActorManaID)(edt + (manaIDRed - drainsManaRed));
			(&a->_effectiveStats.redMana)[aType] =
				clamp(
					0,
					(&a->_effectiveStats.redMana)[aType] - amt,
					(&(a->getBaseStats())->redMana)[aType]);
		}
		break;
	case drainsLifeLevel:
		{
			int16 &maxVit = (a->getBaseStats())->vitality;
			maxVit = clamp(0, maxVit - amt, absoluteMaximumVitality);
			a->acceptDamage(cst->thisID(), amt > 0 ? 1 : -1, kDamageOther);
		}
		break;
	case drainsVitality:
		a->acceptDamage(cst->thisID(), amt, kDamageOther);
		break;
	default:
		break;
	}
	updateIndicators();
}

void ProtoDrainage::implement(GameObject *cst, SpellTarget *trg, int8) {
	int8 totalDice;
	Actor *a;
	Actor *ac;
	if (isActor(cst)) {
		ac = (Actor *) cst;
		totalDice = dice + skillDice * ac->getStats()->spellcraft;
		if (totalDice > 0 && trg->getObject() && isActor(trg->getObject()))
			offensiveNotification(ac, (Actor *) trg->getObject());
	} else {
		ac = nullptr;
		totalDice = dice + 6;
		ObjectID pID = cst->possessor();
		if (pID != Nothing) {
			Actor *p = (Actor *) GameObject::objectAddress(pID);
			assert(isActor(p));
			if (totalDice > 0 && trg->getObject() && isActor(trg->getObject()))
				offensiveNotification(p, (Actor *) trg->getObject());
		}
	}
	int8 totalDamage = diceRoll(totalDice, 6, 0, 0);


	if (!(trg->getType() == SpellTarget::spellTargetObject))
		return;
	GameObject *target = self ? cst : trg->getObject();
	if (!isActor(target))
		return;
	a = (Actor *) target;
	if (a->hasEffect(actorNoDrain))
		return;

	if (totalDamage > 0 && target->makeSavingThrow())
		totalDamage /= 2;
	totalDamage = clamp(0, totalDamage, currentLevel(a, type));

	drainLevel(cst, a, type, totalDamage);
	if (ac != nullptr)
		drainLevel(cst, ac, type, -totalDamage);
}

// ------------------------------------------------------------------
// enchant something based on an enchantment proto-effect

bool ProtoEnchantment::realSavingThrow(Actor *a) {
	uint32 power = (a->getBaseStats())->vitality;
	power *= power;
	int32 saveSpace = absoluteMaximumVitality;
	saveSpace *= saveSpace;
	return (g_vm->_rnd->getRandomNumber(saveSpace - 1) < power);

}

void ProtoEnchantment::implement(GameObject *cst, SpellTarget *trg, int8) {
	if (isActor(trg->getObject())) {
		// can someone be angry at a wand?
		if (isHarmful(enchID)) {
			if (isActor(cst)) {
				offensiveNotification((Actor *) cst, (Actor *) trg->getObject());
			} else {
				ObjectID pID = cst->possessor();
				if (pID != Nothing) {
					Actor *p = (Actor *) GameObject::objectAddress(pID);
					assert(isActor(p));
					offensiveNotification(p, (Actor *) trg->getObject());
				}
			}
		}


		if (((Actor *)(trg->getObject()))->hasEffect(actorNoEnchant) &&
		        isHarmful(enchID))
			return;
		if (canFail() && realSavingThrow((Actor *)(trg->getObject())))
			return;
	}

	if (isHarmful(enchID) && trg->getObject()->makeSavingThrow())
		return;
	EnchantObject(trg->getObject()->thisID(), enchID, minEnch + dice.roll());
}

// ------------------------------------------------------------------
// effects on TAGs

void ProtoTAGEffect::implement(GameObject *cst, SpellTarget *trg, int8) {
	ActiveItem *tag = trg->getTAG();
	assert(tag);
	if (affectBit == settagLocked) {
		//if ( tag->builtInBehavior()==ActiveItem::builtInDoor )
		if (tag->isLocked() != onOff)
			tag->acceptLockToggle(cst->thisID(), tag->lockType());
	} else if (affectBit == settagOpen) {
		tag->trigger(cst->thisID(), onOff);
	}
}

// ------------------------------------------------------------------
// effects on non-Actors

void ProtoObjectEffect::implement(GameObject *, SpellTarget *trg, int8) {
	GameObject *go = trg->getObject();
	assert(go);
	if (!isActor(go))
		EnchantObject(go->thisID(), affectBit, dice.roll());
}

// ------------------------------------------------------------------
// effects on TAGs

void ProtoLocationEffect::implement(GameObject *, SpellTarget *, int8) {
	//TilePoint tp=trg->getPoint();

}

// ------------------------------------------------------------------
// use a special spell on something

void ProtoSpecialEffect::implement(GameObject *cst, SpellTarget *trg, int8) {
	assert(handler);
	(*handler)(cst, trg);
}

/* ===================================================================== *
   Effect Applicability
 * ===================================================================== */

bool ProtoDamage::applicable(SpellTarget &trg) {
	return trg.getType() == SpellTarget::spellTargetObject ||
	       trg.getType() == SpellTarget::spellTargetObjectPoint;
}

bool ProtoDrainage::applicable(SpellTarget &trg) {
	return (trg.getType() == SpellTarget::spellTargetObject ||
	        trg.getType() == SpellTarget::spellTargetObjectPoint) &&
	       isActor(trg.getObject());
}

bool ProtoEnchantment::applicable(SpellTarget &trg) {
	return (trg.getType() == SpellTarget::spellTargetObject ||
	        trg.getType() == SpellTarget::spellTargetObjectPoint) &&
	       (isActor(trg.getObject()) ||
	        getEnchantmentSubType(enchID) == actorInvisible);
}

bool ProtoTAGEffect::applicable(SpellTarget &trg) {
	return (trg.getType() == SpellTarget::spellTargetTAG);
}

bool ProtoObjectEffect::applicable(SpellTarget &trg) {
	return (trg.getType() == SpellTarget::spellTargetObject ||
	        trg.getType() == SpellTarget::spellTargetObjectPoint) &&
	       !isActor(trg.getObject());
}

// ------------------------------------------------------------------
// These are effects for specific spells

#ifdef __WATCOMC__
#pragma off (unreferenced);
#endif

void createSpellCallFrame(GameObject *go, SpellTarget *trg, scriptCallFrame &scf) {
	assert(go);
	assert(trg);
	scf.invokedObject = Nothing;
	scf.enactor       = go->thisID();
	scf.directObject  = Nothing;
	scf.directTAI     = NoActiveItem;
	scf.coords        = Nowhere;

	switch (trg->getType()) {
	case SpellTarget::spellTargetPoint      :
	case SpellTarget::spellTargetObjectPoint:
		scf.value = 1;
		scf.coords = trg->getPoint();
		break;
	case SpellTarget::spellTargetObject     :
		scf.value = 2;
		scf.directObject = trg->getObject()->thisID();
		break;
	case SpellTarget::spellTargetTAG        :
		scf.value = 3;
		scf.directTAI = trg->getTAG()->thisID();
		break;
	case SpellTarget::spellTargetNone       :
	default                                 :
		scf.value = 0;
		break;
	}

}



SPECIALSPELL(CreateFireWisp) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateFireWisp, scf);
}

SPECIALSPELL(CreateWindWisp) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateWindWisp, scf);
}

SPECIALSPELL(CreateWraith) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateWraith, scf);
}

SPECIALSPELL(TeleportToShrine) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_TeleportToShrine, scf);
}

SPECIALSPELL(TeleportToLocation) {
	cst->move(trg->getPoint());
}

SPECIALSPELL(Rejoin) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_Rejoin, scf);
}

SPECIALSPELL(CreateWWisp) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateWindWisp, scf);
}

SPECIALSPELL(CreateFWisp) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateFireWisp, scf);
}

SPECIALSPELL(CreateFood) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_CreateFood, scf);
}

SPECIALSPELL(Timequake) {
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_Timequake, scf);
}

SPECIALSPELL(SagaSpellCall) {


}

SPECIALSPELL(DeathSpell) {
	// can someone be angry at a wand?
	if (isActor(trg->getObject())) {
		if (isActor(cst)) {
			offensiveNotification((Actor *) cst, (Actor *) trg->getObject());
		} else {
			ObjectID pID = cst->possessor();
			if (pID != Nothing) {
				Actor *p = (Actor *) GameObject::objectAddress(pID);
				assert(isActor(p));
				offensiveNotification(p, (Actor *) trg->getObject());
			}
		}
		if (ProtoEnchantment::realSavingThrow((Actor *)(trg->getObject())))
			return;
		Actor *a = (Actor *) trg->getObject();
		if (!a->makeSavingThrow()) {
			a->acceptDamage(cst->thisID(),
			                a->_effectiveStats.vitality,
			                kDamageEnergy, 1, 2, 0);
			a->die();
		}
	}
}

SPECIALSPELL(Resurrect) {
#if 0
	if (isActor(trg->getObject())) {
		Actor *a = (Actor *) trg->getObject();
		if (a->isDead()) a->imNotQuiteDead();
	}
#else
	scriptCallFrame scf;
	createSpellCallFrame(cst, trg, scf);
	runScript(resImports->EXP_spellEffect_Timequake, scf);
#endif
}

SPECIALSPELL(DispellPoison) {
	if (isActor(trg->getObject())) {
		Actor *a = (Actor *) trg->getObject();
		DispelObjectEnchantment(a->thisID(), makeEnchantmentID(actorPoisoned, true));

	}
}

SPECIALSPELL(DispellProtections) {
	if (isActor(trg->getObject())) {
		Actor               *a = (Actor *) trg->getObject();
		GameObject          *obj;
		ContainerIterator   iter(a);

		if (isActor(cst)) {
			offensiveNotification((Actor *) cst, (Actor *) trg->getObject());
		} else {
			ObjectID pID = cst->possessor();
			if (pID != Nothing) {
				Actor *p = (Actor *) GameObject::objectAddress(pID);
				assert(isActor(p));
				offensiveNotification(p, (Actor *) trg->getObject());
			}
		}

		if (ProtoEnchantment::realSavingThrow((Actor *)(trg->getObject())))
			return;

		clearEnchantments(a);

		while (iter.next(&obj) != Nothing) {
			ProtoObj *proto = obj->proto();

			if (proto->containmentSet() & ProtoObj::isEnchantment) {
				uint16 enchantmentID = obj->getExtra();
				if (!isHarmful(enchantmentID)) {
					DispelObjectEnchantment(a->thisID(), enchantmentID);
				}
			}
		}
	}
}

SPECIALSPELL(DispellCurses) {
	if (isActor(trg->getObject())) {
		Actor               *a = (Actor *) trg->getObject();
		GameObject          *obj;
		ContainerIterator   iter(a);
		GameObject          *ToBeDeleted = nullptr;

		clearEnchantments(a);

		while (iter.next(&obj) != Nothing) {
			ProtoObj *proto = obj->proto();

			if (proto->containmentSet() & ProtoObj::isEnchantment) {
				uint16 enchantmentID = obj->getExtra();
				if (isHarmful(enchantmentID)) {
					if (ToBeDeleted) ToBeDeleted->deleteObject();
					ToBeDeleted = obj;
				}
			}
		}
		if (ToBeDeleted) ToBeDeleted->deleteObject();

		a->evalEnchantments();
	}
}

} // end of namespace Saga2
