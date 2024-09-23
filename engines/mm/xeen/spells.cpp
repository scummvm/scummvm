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
 */

#include "mm/xeen/spells.h"
#include "mm/xeen/dialogs/dialogs_items.h"
#include "mm/xeen/dialogs/dialogs_spells.h"
#include "mm/xeen/files.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

Spells::Spells(XeenEngine *vm) : _vm(vm) {
	_lastCaster = 0;

	load();
}

void Spells::load() {
	File f1((g_vm->getGameID() == GType_Clouds) ? "spells.cld" : "spells.xen", 1);
	int i = 0;
	while (f1.pos() < f1.size()) {
		if (Common::RU_RUS == g_vm->getLanguage() && GType_Clouds == g_vm->getGameID()) {
			f1.readString();
			_spellNames.push_back(Res.CLOUDS_SPELLS[i++]);
		} else 
			_spellNames.push_back(f1.readString());

	}
	f1.close();
}

int Spells::calcSpellCost(int spellId, int expenseFactor) const {
	int amount = Res.SPELL_COSTS[spellId];
	return (amount >= 0) ? (amount * 100) << expenseFactor :
		(amount * -500) << expenseFactor;
}

int Spells::calcSpellPoints(int spellId, int expenseFactor) const {
	int amount = Res.SPELL_COSTS[spellId];
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

	if (spellId < 76)
		(this->*SPELL_LIST[spellId])();
}

void Spells::spellFailed() {
	ErrorScroll::show(_vm, Res.SPELL_FAILED, WT_NONFREEZED_WAIT);
}

void Spells::castItemSpell(int itemSpellId) {
	assert(itemSpellId != 0);

	switch (itemSpellId) {
	case 16:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Jump);
			return;
		}
		break;
	case 21:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_WizardEye);
			return;
		}
		break;
	case 28:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_LloydsBeacon);
			return;
		}
		break;
	case 33:
		frostbite2();
		break;
	case 42:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Teleport);
			return;
		}
		break;
	case 48:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_SuperShelter);
			return;
		}
		break;
	case 55:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_TownPortal);
			return;
		}
		break;
	case 58:
		if (_vm->_mode == MODE_COMBAT) {
			NotWhileEngaged::show(_vm, MS_Etheralize);
			return;
		}
		break;
	default:
		break;
	}

	static const MagicSpell spells[74] = {
		NO_SPELL, MS_Light, MS_Awaken, MS_MagicArrow, MS_FirstAid, MS_FlyingFist,
		MS_EnergyBlast, MS_Sleep, MS_Revitalize, MS_CureWounds, MS_Sparks,
		MS_Shrapmetal, MS_InsectSpray, MS_ToxicCloud, MS_ProtFromElements, MS_Pain,
		MS_Jump, MS_BeastMaster, MS_Clairvoyance, MS_TurnUndead, MS_Levitate,
		MS_WizardEye, MS_Bless, MS_IdentifyMonster, MS_LightningBolt, MS_HolyBonus,
		MS_PowerCure, MS_NaturesCure, MS_LloydsBeacon, MS_PowerShield, MS_Heroism,
		MS_Hynotize, MS_WalkOnWater, NO_SPELL, MS_DetectMonster, MS_Fireball,
		MS_ColdRay, MS_CurePoison, MS_AcidSpray, MS_TimeDistortion, MS_DragonSleep,
		MS_CureDisease, MS_Teleport, MS_FingerOfDeath, MS_CureParalysis, MS_GolemStopper,
		MS_PoisonVolley, MS_DeadlySwarm, MS_SuperShelter, MS_DayOfProtection, MS_DayOfSorcery,
		MS_CreateFood, MS_FieryFlail, MS_RechargeItem, MS_FantasticFreeze, MS_TownPortal,
		MS_StoneToFlesh, MS_RaiseDead, MS_Etheralize, MS_DancingSword, MS_MoonRay,
		MS_MassDistortion, MS_PrismaticLight, MS_EnchantItem, MS_Incinerate, MS_HolyWord,
		MS_Resurrection, MS_ElementalStorm, MS_MegaVolts, MS_Inferno, MS_SunRay,
		MS_Implosion, MS_StarBurst, MS_DivineIntervention
	};

	executeSpell(spells[itemSpellId]);
}

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

int Spells::subSpellCost(Character &c, int spellId) {
	Party &party = *_vm->_party;
	int gemCost = Res.SPELL_GEM_COST[spellId];
	int spCost = Res.SPELL_COSTS[spellId];

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

void Spells::addSpellCost(Character &c, int spellId) {
	Party &party = *_vm->_party;
	int gemCost = Res.SPELL_GEM_COST[spellId];
	int spCost = Res.SPELL_COSTS[spellId];

	if (spCost < 1)
		spCost *= -1 * c.getCurrentLevel();

	c._currentSp += spCost;
	party._gems += gemCost;
}

void Spells::acidSpray() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 15;
	combat._damageType = DT_POISON;
	combat._rangeType = RT_ALL;
	sound.playFX(17);
	combat.rangedAttack(POW_SPRAY);
}

void Spells::awaken() {
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		Character &c = party._activeParty[idx];
		c._conditions[ASLEEP] = 0;
		if (c._currentHp > 0)
			c._conditions[UNCONSCIOUS] = 0;
	}

	intf.drawParty(true);
	sound.playFX(30);
}

void Spells::beastMaster() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_BEASTMASTER;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_MAGIC_ORB);
}

void Spells::bless() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	sound.playFX(30);
	party._blessed = combat._oldCharacter->getCurrentLevel();
}

void Spells::clairvoyance() {
	_vm->_party->_clairvoyanceActive = true;
	_vm->_sound->playFX(20);
}

void Spells::coldRay() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = _vm->getRandomNumber(2, 4) * combat._oldCharacter->getCurrentLevel();
	combat._damageType = DT_COLD;
	combat._rangeType = RT_ALL;
	sound.playFX(15);
	combat.rangedAttack(POW_COLD_RAY);
}

void Spells::createFood() {
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	party._food += party._activeParty.size();
	sound.playFX(20);
}

void Spells::cureDisease() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_CureDisease);
	if (!c)
		return;

	sound.playFX(30);
	c->addHitPoints(0);
	c->_conditions[DISEASED] = 0;
	intf.drawParty(true);
}

void Spells::cureParalysis() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_CureParalysis);
	if (!c)
		return;

	sound.playFX(30);
	c->addHitPoints(0);
	c->_conditions[PARALYZED] = 0;
	intf.drawParty(true);
}

void Spells::curePoison() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_CurePoison);
	if (!c)
		return;

	sound.playFX(30);
	c->addHitPoints(0);
	c->_conditions[POISONED] = 0;
	intf.drawParty(true);
}

void Spells::cureWounds() {
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_CureWounds);
	if (!c)
		return;

	if (c->isDead()) {
		spellFailed();
	} else {
		sound.playFX(30);
		c->addHitPoints(15);
	}
}

void Spells::dancingSword() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = _vm->getRandomNumber(6, 14) * combat._oldCharacter->getCurrentLevel();
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_SPARKLES);
}

void Spells::dayOfProtection() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	int lvl = combat._oldCharacter->getCurrentLevel();
	party._walkOnWaterActive = true;
	party._heroism = lvl;
	party._holyBonus = lvl;
	party._blessed = lvl;
	party._poisonResistance = lvl;
	party._coldResistance = lvl;
	party._electricityResistance = lvl;
	party._fireResistance = lvl;
	party._lightCount = lvl;
	sound.playFX(20);
}

void Spells::dayOfSorcery() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	int lvl = combat._oldCharacter->getCurrentLevel();
	party._powerShield = lvl;
	party._clairvoyanceActive = true;
	party._wizardEyeActive = true;
	party._levitateCount = 1;
	party._lightCount = lvl;
	party._automapOn = false;
	sound.playFX(20);
}

void Spells::deadlySwarm() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 40;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(13);
	combat.rangedAttack(POW_DEADLY_SWARM);
}

void Spells::detectMonster() {
	DetectMonsters::show(_vm);
}

void Spells::divineIntervention() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Character &castChar = *combat._oldCharacter;

	if ((castChar._tempAge + 5) > 250) {
		castChar._tempAge = 250;
	} else {
		castChar._tempAge += 5;
	}

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		Character &c = party._activeParty[idx];
		Common::fill(&c._conditions[CURSED], &c._conditions[ERADICATED], 0);
		if (!c._conditions[ERADICATED])
			c._currentHp = c.getMaxHP();
	}

	sound.playFX(20);
	intf.drawParty(true);
}

void Spells::dragonSleep() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_DRAGONSLEEP;
	combat._rangeType = RT_SINGLE;
	sound.playFX(18);
	combat.rangedAttack(POW_MAGIC_ORB);
}

void Spells::elementalStorm() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;
	static const int STORM_FX_LIST[4] = { 13, 14, 15, 17 };
	static const PowType STORM_MA_LIST[4] = {
		POW_FIREBALL, POW_SPARKS, POW_FROST_WAVE, POW_SPRAY
	};

	combat._monsterDamage = 150;
	combat._damageType = (DamageType)_vm->getRandomNumber(DT_FIRE, DT_POISON);
	combat._rangeType = RT_ALL;
	sound.playFX(STORM_FX_LIST[combat._damageType - DT_FIRE]);
	combat.rangedAttack(STORM_MA_LIST[combat._damageType - DT_FIRE]);
}

void Spells::enchantItem() {
	Mode oldMode = _vm->_mode;

	Character *c = SpellOnWho::show(_vm, MS_EnchantItem);
	if (!c)
		return;

	ItemsDialog::show(_vm, c, ITEMMODE_ENCHANT);

	_vm->_mode = oldMode;
}

void Spells::energyBlast() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = combat._oldCharacter->getCurrentLevel() * 2;
	combat._damageType = DT_ENERGY;
	combat._rangeType = RT_SINGLE;
	sound.playFX(16);
	combat.rangedAttack(POW_ENERGY_BLAST);
}

void Spells::etherialize() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Common::Point pt = party._mazePosition + Common::Point(
		Res.SCREEN_POSITIONING_X[party._mazeDirection][7],
		Res.SCREEN_POSITIONING_Y[party._mazeDirection][7]
	);

	if ((map.mazeData()._mazeFlags & RESTRICTION_ETHERIALIZE) ||
			map.mazeLookup(pt, 0, 0xffff) == INVALID_CELL) {
		spellFailed();
	} else {
		party._mazePosition = pt;
		sound.playFX(51);
	}
}

void Spells::fantasticFreeze() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 40;
	combat._damageType = DT_COLD;
	combat._rangeType = RT_GROUP;
	sound.playFX(15);
	combat.rangedAttack(POW_COLD_RAY);
}

void Spells::fieryFlail() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 100;
	combat._damageType = DT_FIRE;
	combat._rangeType = RT_SINGLE;
	sound.playFX(13);
	combat.rangedAttack(POW_FIERY_FLAIL);
}

void Spells::fingerOfDeath() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_FINGEROFDEATH;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_SPARKLES);
}

void Spells::fireball() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = _vm->getRandomNumber(3, 7) * combat._oldCharacter->getCurrentLevel();
	combat._damageType = DT_FIRE;
	combat._rangeType = RT_GROUP;
	sound.playFX(13);
	combat.rangedAttack(POW_FIREBALL);
}

void Spells::firstAid() {
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_FirstAid);
	if (!c)
		return;

	if (c->isDead()) {
		spellFailed();
	} else {
		sound.playFX(30);
		c->addHitPoints(6);
	}
}

void Spells::flyingFist() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 6;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_SINGLE;
	sound.playFX(18);
	combat.rangedAttack(POW_SPARKLES);
}

void Spells::frostbite() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 35;
	combat._damageType = DT_COLD;
	combat._rangeType = RT_SINGLE;
	sound.playFX(8);
	combat.rangedAttack(POW_COLD_RAY);
}

void Spells::golemStopper() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_GOLEMSTOPPER;
	combat._rangeType = RT_SINGLE;
	sound.playFX(16);
	combat.rangedAttack(POW_STOPPER);
}

void Spells::heroism() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	sound.playFX(30);
	party._heroism = combat._oldCharacter->getCurrentLevel();
}

void Spells::holyBonus() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	sound.playFX(30);
	party._holyBonus = combat._oldCharacter->getCurrentLevel();
}

void Spells::holyWord() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_HOLYWORD;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_ENERGY_BLAST);
}

void Spells::hypnotize() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_HYPNOTIZE;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_MAGIC_ORB);
}

void Spells::identifyMonster() {
	Combat &combat = *_vm->_combat;

	if (combat._attackMonsters[0] == -1 && combat._attackMonsters[1] == -1
		&& combat._attackMonsters[2] == -1) {
		spellFailed();
	} else {
		IdentifyMonster::show(_vm);
	}
}

void Spells::implosion() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 1000;
	combat._damageType = DT_ENERGY;
	combat._rangeType = RT_SINGLE;
	sound.playFX(18);
	combat.rangedAttack(POW_STOPPER);
}

void Spells::incinerate() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 250;
	combat._damageType = DT_FIRE;
	combat._rangeType = RT_SINGLE;
	sound.playFX(22);
	combat.rangedAttack(POW_INCINERATE);
}

void Spells::inferno() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 250;
	combat._damageType = DT_FIRE;
	combat._rangeType = RT_GROUP;
	sound.playFX(13);
	combat.rangedAttack(POW_INCINERATE);
}

void Spells::insectSpray() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_INSECT_SPRAY;
	combat._rangeType = RT_GROUP;
	sound.playFX(17);
	combat.rangedAttack(POW_SPRAY);
}

void Spells::itemToGold() {
	Windows &windows = *_vm->_windows;
	Character *c = SpellOnWho::show(_vm, MS_ItemToGold);
	if (!c)
		return;

	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_FF;

	windows[11].close();
	ItemsDialog::show(_vm, c, ITEMMODE_TO_GOLD);

	_vm->_mode = oldMode;
}

void Spells::jump() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (map._isOutdoors) {
		map.getCell(7);
		if (map._currentWall != 1) {
			map.getCell(14);
			if (map._currentSurfaceId != 0 && map._currentWall != 1) {
				party._mazePosition += Common::Point(
					Res.SCREEN_POSITIONING_X[party._mazeDirection][14],
					Res.SCREEN_POSITIONING_Y[party._mazeDirection][14]
					);
				sound.playFX(51);
				party._stepped = true;
				return;
			}
		}
	} else {
		Common::Point pt = party._mazePosition + Common::Point(
			Res.SCREEN_POSITIONING_X[party._mazeDirection][7],
			Res.SCREEN_POSITIONING_Y[party._mazeDirection][7]);
		if (!map.mazeLookup(party._mazePosition, Res.MONSTER_GRID_BITMASK[party._mazeDirection]) &&
			!map.mazeLookup(pt, Res.MONSTER_GRID_BITMASK[party._mazeDirection])) {
			party._mazePosition += Common::Point(
				Res.SCREEN_POSITIONING_X[party._mazeDirection][14],
				Res.SCREEN_POSITIONING_Y[party._mazeDirection][14]
			);
			sound.playFX(51);
			party._stepped = true;
			return;
		}
	}

	spellFailed();
}

void Spells::levitate() {
	_vm->_party->_levitateCount = 1;
	_vm->_sound->playFX(20);
}

void Spells::light() {
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	++party._lightCount;
	if (intf._obscurity != OBSCURITY_BLACK)
		party._stepped = true;
	sound.playFX(39);
}

void Spells::lightningBolt() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = _vm->getRandomNumber(4, 6) * combat._oldCharacter->getCurrentLevel();
	combat._damageType = DT_ELECTRICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(14);
	combat.rangedAttack(POW_LIGHTNING);
}

void Spells::lloydsBeacon() {
	if (_vm->_map->mazeData()._mazeFlags & RESTRICTION_LLOYDS_BEACON) {
		spellFailed();
	} else {
		if (!LloydsBeacon::show(_vm))
			spellFailed();
	}
}

void Spells::magicArrow() {
	Combat &combat = *_vm->_combat;
	combat._monsterDamage = 0;
	combat._damageType = DT_MAGIC_ARROW;
	combat._rangeType = RT_SINGLE;
	combat.rangedAttack(POW_ARROW);
}

void Spells::massDistortion() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_MASS_DISTORTION;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_STOPPER);
}

void Spells::megaVolts() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 150;
	combat._damageType = DT_ELECTRICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(14);
	combat.rangedAttack(POW_MEGAVOLTS);
}

void Spells::moonRay() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 30;
	combat._damageType = DT_ENERGY;
	combat._rangeType = RT_ALL;
	sound.playFX(16);
	combat.rangedAttack(POW_ENERGY_BLAST);

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		sound.playFX(30);
		party._activeParty[idx].addHitPoints(_vm->getRandomNumber(1, 30));
	}

	intf.drawParty(true);
}

void Spells::naturesCure() {
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_NaturesCure);
	if (!c)
		return;

	if (c->isDead()) {
		spellFailed();
	} else {
		sound.playFX(30);
		c->addHitPoints(25);
	}
}

void Spells::pain() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_SPARKLES);
}

void Spells::poisonVolley() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 10;
	combat._damageType = DT_POISON_VOLLEY;
	combat._rangeType = RT_ALL;
	sound.playFX(49);
	combat.rangedAttack(POW_ARROW);
}

void Spells::powerCure() {
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_PowerCure);
	if (!c)
		return;

	if (c->isDead()) {
		spellFailed();
	} else {
		sound.playFX(30);
		c->addHitPoints(_vm->getRandomNumber(2, 12) * _vm->_combat->_oldCharacter->getCurrentLevel());
	}
}

void Spells::powerShield() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	sound.playFX(20);
	party._powerShield = combat._oldCharacter->getCurrentLevel();
}

void Spells::prismaticLight() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 80;
	combat._damageType = (DamageType)_vm->getRandomNumber(DT_PHYSICAL, DT_ENERGY);
	combat._rangeType = RT_ALL;
	sound.playFX(18);
	combat.rangedAttack(POW_SPARKLES);
}

void Spells::protectionFromElements() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Character &c = *combat._oldCharacter;
	int resist = MIN(c.getCurrentLevel() * 2 + 5, (uint)200);

	int elementType = SelectElement::show(_vm, MS_ProtFromElements);
	if (elementType != -1) {
		switch (elementType) {
		case DT_FIRE:
			party._fireResistance = resist;
			break;
		case DT_ELECTRICAL:
			party._fireResistance = resist;
			break;
		case DT_COLD:
			party._coldResistance = resist;
			break;
		case DT_POISON:
			party._poisonResistance = resist;
			break;
		default:
			break;
		}

		sound.playFX(20);
		intf.drawParty(true);
	}
}

void Spells::raiseDead() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_RaiseDead);
	if (!c)
		return;

	if (!c->_conditions[DEAD]) {
		spellFailed();
	} else {
		c->_conditions[DEAD] = 0;
		c->_conditions[UNCONSCIOUS] = 0;
		c->_currentHp = 0;
		sound.playFX(30);
		c->addHitPoints(1);
		if (--c->_endurance._permanent < 1)
			c->_endurance._permanent = 1;

		intf.drawParty(true);
	}
}

void Spells::rechargeItem() {
	Mode oldMode = _vm->_mode;

	Character *c = SpellOnWho::show(_vm, MS_RechargeItem);
	if (!c)
		return;

	ItemsDialog::show(_vm, c, ITEMMODE_RECHARGE);
	_vm->_mode = oldMode;
}

void Spells::resurrection() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_RaiseDead);
	if (!c)
		return;

	if (!c->_conditions[ERADICATED]) {
		spellFailed();
		sound.playFX(30);
	} else {
		sound.playFX(30);
		c->addHitPoints(0);
		c->_conditions[ERADICATED] = 0;

		if (--c->_endurance._permanent < 1)
			c->_endurance._permanent = 1;
		if ((c->_tempAge + 5) >= 250)
			c->_tempAge = 250;
		else
			c->_tempAge += 5;

		intf.drawParty(true);
	}
}

void Spells::revitalize() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_Revitalize);
	if (!c)
		return;

	sound.playFX(30);
	c->addHitPoints(0);
	c->_conditions[WEAK] = 0;
	intf.drawParty(true);
}

void Spells::shrapMetal() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = combat._oldCharacter->getCurrentLevel() * 2;
	combat._damageType = DT_PHYSICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(16);
	combat.rangedAttack(POW_DEADLY_SWARM);
}

void Spells::sleep() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_SLEEP;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_MAGIC_ORB);
}

void Spells::sparks() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = combat._oldCharacter->getCurrentLevel() * 2;
	combat._damageType = DT_ELECTRICAL;
	combat._rangeType = RT_GROUP;
	sound.playFX(14);
	combat.rangedAttack(POW_SPARKS);
}

void Spells::starBurst() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 500;
	combat._damageType = DT_FIRE;
	combat._rangeType = RT_ALL;
	sound.playFX(13);
	combat.rangedAttack(POW_DEADLY_SWARM);
}

void Spells::stoneToFlesh() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_StoneToFlesh);
	if (!c)
		return;

	sound.playFX(30);
	c->addHitPoints(0);
	c->_conditions[STONED] = 0;
	intf.drawParty(true);
}

void Spells::sunRay() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 200;
	combat._damageType = DT_ENERGY;
	combat._rangeType = RT_ALL;
	sound.playFX(16);
	combat.rangedAttack(POW_ENERGY_BLAST);
}

void Spells::superShelter() {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Sound &sound = *_vm->_sound;

	if (map.mazeData()._mazeFlags & RESTRICTION_SUPER_SHELTER) {
		spellFailed();
	} else {
		Mode oldMode = _vm->_mode;
		_vm->_mode = MODE_INTERACTIVE2;
		sound.playFX(30);
		intf.rest();
		_vm->_mode = oldMode;
	}
}

void Spells::suppressDisease() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_SuppressDisease);
	if (!c)
		return;

	if (c->_conditions[DISEASED]) {
		if (c->_conditions[DISEASED] >= 4)
			c->_conditions[DISEASED] -= 3;
		else
			c->_conditions[DISEASED] = 1;

		sound.playFX(20);
		c->addHitPoints(0);
		intf.drawParty(true);
	}
}

void Spells::suppressPoison() {
	Interface &intf = *_vm->_interface;
	Sound &sound = *_vm->_sound;

	Character *c = SpellOnWho::show(_vm, MS_FirstAid);
	if (!c)
		return;

	if (c->_conditions[POISONED]) {
		if (c->_conditions[POISONED] >= 4) {
			c->_conditions[POISONED] -= 2;
		} else {
			c->_conditions[POISONED] = 1;
		}
	}

	sound.playFX(20);
	c->addHitPoints(0);
	intf.drawParty(1);
}

void Spells::teleport() {
	Map &map = *_vm->_map;
	Sound &sound = *_vm->_sound;

	if (map.mazeData()._mazeFlags & RESTRICTION_TELPORT) {
		spellFailed();
	} else {
		switch (Teleport::show(_vm)) {
		case 0:
			spellFailed();
			break;
		case 1:
			sound.playFX(51);
			break;
		default:
			break;
		}
	}
}

void Spells::timeDistortion() {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (map.mazeData()._mazeFlags & RESTRICTION_TIME_DISTORTION) {
		spellFailed();
	} else {
		party.moveToRunLocation();
		sound.playFX(51);
		intf.draw3d(true);
	}
}

void Spells::townPortal() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (map.mazeData()._mazeFlags & RESTRICTION_TOWN_PORTAL) {
		spellFailed();
		return;
	}

	int townNumber = TownPortal::show(_vm);
	if (!townNumber)
		return;

	sound.playFX(51);
	map._loadCcNum = map._sideTownPortal;
	_vm->_files->_ccNum = map._sideTownPortal > 0;

	int arrIndex = _vm->getGameID() == GType_Swords ? 2 : map._sideTownPortal;
	map.load(Res.TOWN_MAP_NUMBERS[arrIndex][townNumber - 1]);

	if (_vm->getGameID() == GType_Swords) {
		party._mazePosition = Common::Point(8, 3);
		party._mazeDirection = DIR_NORTH;
	} else if (!_vm->_files->_ccNum) {
		party.moveToRunLocation();
	} else {
		switch (townNumber) {
		case 1:
			party._mazePosition = Common::Point(14, 11);
			party._mazeDirection = DIR_SOUTH;
			break;
		case 2:
			party._mazePosition = Common::Point(5, 12);
			party._mazeDirection = DIR_WEST;
			break;
		case 3:
			party._mazePosition = Common::Point(2, 15);
			party._mazeDirection = DIR_EAST;
			break;
		case 4:
			party._mazePosition = Common::Point(8, 11);
			party._mazeDirection = DIR_NORTH;
			break;
		case 5:
			party._mazePosition = Common::Point(2, 6);
			party._mazeDirection = DIR_NORTH;
			break;
		default:
			break;
		}
	}
}

void Spells::toxicCloud() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 10;
	combat._damageType = DT_POISON;
	combat._rangeType = RT_GROUP;
	sound.playFX(17);
	combat.rangedAttack(POW_SPRAY);
}

void Spells::turnUndead() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 0;
	combat._damageType = DT_UNDEAD;
	combat._rangeType = RT_GROUP;
	sound.playFX(18);
	combat.rangedAttack(POW_ENERGY_BLAST);
}

void Spells::walkOnWater() {
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	party._walkOnWaterActive = true;
	sound.playFX(20);
}

void Spells::wizardEye() {
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	party._wizardEyeActive = true;
	party._automapOn = false;
	sound.playFX(20);
}

void Spells::frostbite2() {
	Combat &combat = *_vm->_combat;
	Sound &sound = *_vm->_sound;

	combat._monsterDamage = 35;
	combat._damageType = DT_COLD;
	combat._rangeType = RT_SINGLE;
	sound.playFX(15);
	combat.rangedAttack(POW_FROST_WAVE);
}

} // End of namespace Xeen
} // End of namespace MM
