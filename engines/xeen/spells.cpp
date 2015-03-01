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
 */

#include "xeen/spells.h"
#include "xeen/dialogs_spells.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Spells::Spells(XeenEngine *vm) : _vm(vm) {
	_lastCaster = 0;

	load();
}

void Spells::load() {
	File f1("spells.xen");
	while (f1.pos() < f1.size())
		_spellNames.push_back(f1.readString());
	f1.close();
}

int Spells::calcSpellCost(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? (amount * 100) << expenseFactor :
		(amount * -500) << expenseFactor;
}

int Spells::calcSpellPoints(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? amount : amount * -1 * expenseFactor;
}

typedef void(Spells::*SpellMethodPtr)();

void Spells::executeSpell(MagicSpell spellId) {
	static const SpellMethodPtr SPELL_LIST[76] = {
		&Spells::acidSpray, &Spells::awaken, &Spells::beastMaster,
		&Spells::bless, &Spells::clairvoyance, &Spells::coldRay,
		&Spells::createFood, &Spells::cureDisease, &Spells::cureParalysis,
		&Spells::curePoison, &Spells::cureWounds, &Spells::dancingSword,
		&Spells::dayOfProtection, &Spells::dayOfSorcery, &Spells::deadlySwarm,
		&Spells::detectMonster, &Spells::divineIntervention, &Spells::dragonSleep,
		&Spells::elementalStorm, &Spells::enchantItem, &Spells::energyBlast,
		&Spells::etherialize, &Spells::fantasticFreeze, &Spells::fieryFlail,
		&Spells::fingerOfDeath, &Spells::fireball, &Spells::firstAid,
		&Spells::flyingFist, &Spells::frostbite, &Spells::golemStopper,
		&Spells::heroism, &Spells::holyBonus, &Spells::holyWord,
		&Spells::hypnotize, &Spells::identifyMonster, &Spells::implosion,
		&Spells::incinerate, &Spells::inferno, &Spells::insectSpray,
		&Spells::itemToGold, &Spells::jump, &Spells::levitate,
		&Spells::light, &Spells::lightningBolt, &Spells::lloydsBeacon,
		&Spells::magicArrow, &Spells::massDistortion, &Spells::megaVolts,
		&Spells::moonRay, &Spells::naturesCure, &Spells::pain,
		&Spells::poisonVolley, &Spells::powerCure, &Spells::powerShield,
		&Spells::prismaticLight, &Spells::protectionFromElements, &Spells::raiseDead,
		&Spells::rechargeItem, &Spells::resurrection, &Spells::revitalize,
		&Spells::shrapMetal, &Spells::sleep, &Spells::sparks,
		&Spells::starBurst, &Spells::stoneToFlesh, &Spells::sunRay,
		&Spells::superShelter, &Spells::suppressDisease, &Spells::suppressPoison,
		&Spells::teleport, &Spells::timeDistortion, &Spells::townPortal,
		&Spells::toxicCloud, &Spells::turnUndead, &Spells::walkOnWater, 
		&Spells::wizardEye
	};

	(this->*SPELL_LIST[spellId])();
}

/**
 * Spell being cast failed
 */
void Spells::spellFailed() {
	ErrorScroll::show(_vm, SPELL_FAILED, WT_NONFREEZED_WAIT);
}

/**
 * Cast a spell associated with an item
 */
void Spells::castItemSpell(int itemSpellId) {
	switch (itemSpellId) {
	case 15:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Jump);
			return;
		}
		break;
	case 20:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_WizardEye);
			return;
		}
		break;
	case 27:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_LloydsBeacon);
			return;
		}
		break;
	case 32:
		frostbite2();
		break;
	case 41:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Teleport);
			return;
		}
		break;
	case 47:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_SuperShelter);
			return;
		}
		break;
	case 54:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_TownPortal);
			return;
		}
		break;
	case 57:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Etheralize);
			return;
		}
		break;
	default:
		break;
	}

	static const MagicSpell spells[73] = {
		MS_Light, MS_Awaken, MS_MagicArrow, MS_FirstAid, MS_FlyingFist,
		MS_EnergyBlast, MS_Sleep, MS_Revitalize, MS_CureWounds, MS_Sparks,
		MS_Shrapmetal, MS_InsectSpray, MS_ToxicCloud, MS_ProtFromElements, MS_Pain,
		MS_Jump, MS_BeastMaster, MS_Clairvoyance, MS_TurnUndead, MS_Levitate,
		MS_WizardEye, MS_Bless, MS_IdentifyMonster, MS_LightningBolt, MS_HolyBonus,
		MS_PowerCure, MS_NaturesCure, MS_LloydsBeacon, MS_PowerShield, MS_Heroism,
		MS_Hynotize, MS_WalkOnWater, NO_SPELL, MS_DetectMonster, MS_Fireball,
		MS_ColdRay, MS_CurePoison, MS_AcidSpray, MS_TimeDistortion, MS_DragonSleep,
		MS_CureDisease, MS_Teleport, MS_FingerOfDeath, MS_CureParalysis, MS_GolemStopper,
		MS_PoisonVolley, MS_DeadlySwarm, MS_SuperShelter, MS_DayOfProtection, MS_DayOfProtection,
		MS_CreateFood, MS_FieryFlail, MS_RechargeItem, MS_FantasticFreeze, MS_TownPortal,
		MS_StoneToFlesh, MS_RaiseDead, MS_Etheralize, MS_DancingSword, MS_MoonRay,
		MS_MassDistortion, MS_PrismaticLight, MS_EnchantItem, MS_Incinerate, MS_HolyWord,
		MS_Resurrection, MS_ElementalStorm, MS_MegaVolts, MS_Inferno, MS_SunRay,
		MS_Implosion, MS_StarBurst, MS_DivineIntervention
	};

	executeSpell(spells[itemSpellId]);
}

/**
 * Cast a given spell
 */
int Spells::castSpell(Character *c, MagicSpell spellId) {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	int oldTillMove = intf._tillMove;
	int result = 1;
	combat._oldCharacter = c;
	
	// Try and subtract the SP and gem requirements for the spell
	int resultError = subSpellCost(*c, spellId);
	if (resultError) {
		CantCast::show(_vm, spellId, resultError);
		result = -1;
	} else {
		// Some spells have special handling
		switch (spellId) {
		case MS_EnchantItem:
		case MS_Etheralize:	
		case MS_Jump:
		case MS_LloydsBeacon:
		case MS_SuperShelter:
		case MS_Teleport:
		case MS_TownPortal:
		case MS_WizardEye:
			if (_vm->_mode != MODE_COMBAT) {
				executeSpell(spellId);
			} else {
				// Return the spell costs and flag that another spell can be selected
				addSpellCost(*c, spellId);
				NotWhileEngaged::show(_vm, spellId);
				result = -1;
			}
			break;

		default:
			executeSpell(spellId);
			break;
		}
	}

	combat._moveMonsters = 1;
	intf._tillMove = oldTillMove;
	return result;
}

/**
 * Subtract the requirements for a given spell if available, returning
 * true if there was sufficient
 */
int Spells::subSpellCost(Character &c, int spellId) {
	Party &party = *_vm->_party;
	int gemCost = SPELL_GEM_COST[spellId];
	int spCost = SPELL_COSTS[spellId];

	// Negative SP costs indicate it's dependent on the character's level
	if (spCost <= 0) {
		spCost = c.getCurrentLevel() * (spCost * -1);
	}

	if (spCost > c._currentSp)
		// Not enough SP
		return 1;
	if (gemCost > (int)party._gems)
		// Not enough gems
		return 2;

	c._currentSp -= spCost;
	party._gems -= gemCost;
	return 0;
}

/**
 * Add the SP and gem requirements for a given spell to the given 
 * character and party
 */
void Spells::addSpellCost(Character &c, int spellId) {
	Party &party = *_vm->_party;
	int gemCost = SPELL_GEM_COST[spellId];
	int spCost = SPELL_COSTS[spellId];

	if (spCost < 1)
		spCost *= -1 * c.getCurrentLevel();

	c._currentSp += spCost;
	party._gems += gemCost;
}

void Spells::acidSpray() { error("TODO: spell"); }

void Spells::awaken() {
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		Character &c = party._activeParty[idx];
		c._conditions[ASLEEP] = 0;
		if (c._currentHp > 0)
			c._conditions[UNCONSCIOUS] = 0;
	}

	intf.drawParty(true);
	sound.playFX(30);
}

void Spells::beastMaster() { error("TODO: spell"); }

void Spells::bless() { error("TODO: spell"); }

void Spells::clairvoyance() { error("TODO: spell"); }

void Spells::coldRay() { error("TODO: spell"); }

void Spells::createFood() { error("TODO: spell"); }

void Spells::cureDisease() { error("TODO: spell"); }

void Spells::cureParalysis() { error("TODO: spell"); }

void Spells::curePoison() { error("TODO: spell"); }

void Spells::cureWounds() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	int charIndex = SpellOnWho::show(_vm, MS_Revitalize);
	if (charIndex == -1)
		return;

	Character &c = combat._combatMode == 2 ? *combat._combatParty[charIndex] :
		party._activeParty[charIndex];

	if (c.isDead()) {
		spellFailed();
	}
	else {
		sound.playFX(30);
		c.addHitPoints(15);
	}
}

void Spells::dancingSword() { error("TODO: spell"); }

void Spells::dayOfProtection() { error("TODO: spell"); }

void Spells::dayOfSorcery() { error("TODO: spell"); }

void Spells::deadlySwarm() { error("TODO: spell"); }

void Spells::detectMonster() { error("TODO: spell"); }

void Spells::divineIntervention() { error("TODO: spell"); }

void Spells::dragonSleep() { error("TODO: spell"); }

void Spells::elementalStorm() { error("TODO: spell"); }

void Spells::enchantItem() { error("TODO: spell"); }

void Spells::energyBlast() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = combat._oldCharacter->getCurrentLevel() * 2;
	combat._damageType = DT_ENERGY;
	combat._rangeType = RT_SINGLE;
	sound.playFX(16);
	combat.multiAttack(13);
}

void Spells::etherialize() { error("TODO: spell"); }		// Not while engaged

void Spells::fantasticFreeze() { error("TODO: spell"); }

void Spells::fieryFlail() { error("TODO: spell"); }

void Spells::fingerOfDeath() { error("TODO: spell"); }

void Spells::fireball() { error("TODO: spell"); }

void Spells::firstAid() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	int charIndex = SpellOnWho::show(_vm, MS_FirstAid);
	if (charIndex == -1)
		return;

	Character &c = combat._combatMode == 2 ? *combat._combatParty[charIndex] :
		party._activeParty[charIndex];

	if (c.isDead()) {
		spellFailed();
	}
	else {
		sound.playFX(30);
		c.addHitPoints(6);
	}
}

void Spells::flyingFist() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = 6;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_SINGLE;
	sound.playFX(18);
	combat.multiAttack(14);
}

void Spells::frostbite() { error("TODO: spell"); }

void Spells::golemStopper() { error("TODO: spell"); }

void Spells::heroism() { error("TODO: spell"); }

void Spells::holyBonus() { error("TODO: spell"); }

void Spells::holyWord() { error("TODO: spell"); }

void Spells::hypnotize() { error("TODO: spell"); }

void Spells::identifyMonster() { error("TODO: spell"); }

void Spells::implosion() { error("TODO: spell"); }

void Spells::incinerate() { error("TODO: spell"); }

void Spells::inferno() { error("TODO: spell"); }

void Spells::insectSpray() { error("TODO: spell"); }

void Spells::itemToGold() { error("TODO: spell"); }

void Spells::jump() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	if (map._isOutdoors) {
		map.getCell(7);
		if (map._currentWall != 1) {
			map.getCell(14);
			if (map._currentSurfaceId != 0 && map._currentWall != 1) {
				party._mazePosition += Common::Point(
					SCREEN_POSITIONING_X[party._mazeDirection][14],
					SCREEN_POSITIONING_Y[party._mazeDirection][14]
					);
				sound.playFX(51);
				party._stepped = true;
				return;
			}
		}
	} else {
		Common::Point pt = party._mazePosition + Common::Point(
			SCREEN_POSITIONING_X[party._mazeDirection][7],
			SCREEN_POSITIONING_Y[party._mazeDirection][7]);
		if (!map.mazeLookup(party._mazePosition, MONSTER_GRID_BITMASK[party._mazeDirection]) &&
			!map.mazeLookup(pt, MONSTER_GRID_BITMASK[party._mazeDirection])) {
			party._mazePosition += Common::Point(
				SCREEN_POSITIONING_X[party._mazeDirection][14],
				SCREEN_POSITIONING_Y[party._mazeDirection][14]
			);
			sound.playFX(51);
			party._stepped = true;
			return;
		}
	}

	spellFailed();
}

void Spells::levitate() { error("TODO: spell"); }

void Spells::light() { 
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	++party._lightCount;
	if (intf._intrIndex1)
		party._stepped = true;
	sound.playFX(39);
}

void Spells::lightningBolt() { error("TODO: spell"); }

void Spells::lloydsBeacon() { error("TODO: spell"); }	// Not while engaged

void Spells::magicArrow() { 
	Combat &combat = *_vm->_combat;
	combat._monsterDamage = 0;
	combat._damageType = DT_MAGIC_ARROW;
	combat._rangeType = RT_SINGLE;
	combat.multiAttack(11);
}

void Spells::massDistortion() { error("TODO: spell"); }

void Spells::megaVolts() { error("TODO: spell"); }

void Spells::moonRay() { error("TODO: spell"); }

void Spells::naturesCure() { error("TODO: spell"); }

void Spells::pain() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.multiAttack(14);
}

void Spells::poisonVolley() { error("TODO: spell"); }

void Spells::powerCure() { error("TODO: spell"); }

void Spells::powerShield() { error("TODO: spell"); }

void Spells::prismaticLight() { error("TODO: spell"); }

void Spells::protectionFromElements() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;
	Character &c = *combat._oldCharacter;
	int resist = MIN(c.getCurrentLevel() * 2 + 5, (uint)200);
	
	int elementType = SelectElement::show(_vm, MS_ProtFromElements);
	if (elementType != -1) {
		switch (elementType) {
		case DT_FIRE:
			party._fireResistence = resist;
			break;
		case DT_ELECTRICAL:
			party._fireResistence = resist;
			break;
		case DT_COLD:
			party._coldResistence = resist;
			break;
		case DT_POISON:
			party._poisonResistence = resist;
			break;
		default:
			break;
		}

		sound.playFX(20);
		intf.drawParty(true);
	}
}

void Spells::raiseDead() { error("TODO: spell"); }

void Spells::rechargeItem() { error("TODO: spell"); }

void Spells::resurrection() { error("TODO: spell"); }

void Spells::revitalize() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	int charIndex = SpellOnWho::show(_vm, MS_Revitalize);
	if (charIndex == -1)
		return;

	Character &c = combat._combatMode == 2 ? *combat._combatParty[charIndex] :
		party._activeParty[charIndex];

	sound.playFX(30);
	c.addHitPoints(0);
	c._conditions[WEAK] = 0;
	intf.drawParty(true);
}

void Spells::shrapMetal() { error("TODO: spell"); }

void Spells::sleep() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_SLEEP;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.multiAttack(7);
}

void Spells::sparks() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = combat._oldCharacter->getCurrentLevel() * 2;
	combat._damageType = DT_ELECTRICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(14);
	combat.multiAttack(5);
}

void Spells::starBurst() { error("TODO: spell"); }

void Spells::stoneToFlesh() { error("TODO: spell"); }

void Spells::sunRay() { error("TODO: spell"); }

void Spells::superShelter() { error("TODO: spell"); }

void Spells::suppressDisease() { error("TODO: spell"); }

void Spells::suppressPoison() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	int charIndex = SpellOnWho::show(_vm, MS_FirstAid);
	if (charIndex == -1)
		return;

	Character &c = combat._combatMode == 2 ? *combat._combatParty[charIndex] :
		party._activeParty[charIndex];

	if (c._conditions[POISONED]) {
		if (c._conditions[POISONED] >= 4) {
			c._conditions[POISONED] -= 2;
		} else {
			c._conditions[POISONED] = 1;
		}
	}

	sound.playFX(20);
	c.addHitPoints(0);
	intf.drawParty(1);
}

void Spells::teleport() { error("TODO: spell"); }		// Not while engaged

void Spells::timeDistortion() { error("TODO: spell"); }

void Spells::townPortal() { error("TODO: spell"); }		// Not while engaged

void Spells::toxicCloud() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = 10;
	combat._damageType = DT_POISON;
	combat._rangeType = RT_GROUP;
	sound.playFX(17);
	combat.multiAttack(10);
}

void Spells::turnUndead() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_UNDEAD;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.multiAttack(13);
}

void Spells::walkOnWater() { error("TODO: spell"); }

void Spells::wizardEye() { error("TODO: spell"); }		// Not while engaged

void Spells::frostbite2() {
	Combat &combat = *_vm->_combat;
	SoundManager &sound = *_vm->_sound;
	
	combat._monsterDamage = 35;
	combat._damageType = DT_COLD;
	combat._rangeType = RT_SINGLE;
	sound.playFX(15);
	combat.multiAttack(9);
}

} // End of namespace Xeen
