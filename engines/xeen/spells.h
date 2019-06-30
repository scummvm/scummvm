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

#ifndef XEEN_SPELLS_H
#define XEEN_SPELLS_H

#include "common/scummsys.h"
#include "common/str-array.h"

namespace Xeen {

class XeenEngine;
class Character;

enum MagicSpell {
	MS_AcidSpray = 0, MS_Awaken = 1, MS_BeastMaster = 2, MS_Bless = 3,
	MS_Clairvoyance = 4,  MS_ColdRay = 5, MS_CreateFood = 6,
	MS_CureDisease = 7, MS_CureParalysis = 8, MS_CurePoison = 9,
	MS_CureWounds = 10, MS_DancingSword = 11, MS_DayOfProtection = 12,
	MS_DayOfSorcery = 13, MS_DeadlySwarm = 14, MS_DetectMonster = 15,
	MS_DivineIntervention = 16, MS_DragonSleep = 17, MS_ElementalStorm = 18,
	MS_EnchantItem = 19, MS_EnergyBlast = 20, MS_Etheralize = 21,
	MS_FantasticFreeze = 22, MS_FieryFlail = 23, MS_FingerOfDeath = 24,
	MS_Fireball = 25, MS_FirstAid = 26, MS_FlyingFist = 27,
	MS_FrostBite = 28, MS_GolemStopper = 29, MS_Heroism = 30,
	MS_HolyBonus = 31, MS_HolyWord = 32, MS_Hynotize = 33,
	MS_IdentifyMonster = 34, MS_Implosion = 35, MS_Incinerate = 36,
	MS_Inferno = 37, MS_InsectSpray = 38, MS_ItemToGold = 39,
	MS_Jump = 40, MS_Levitate = 41, MS_Light = 42, MS_LightningBolt = 43,
	MS_LloydsBeacon = 44, MS_MagicArrow = 45, MS_MassDistortion = 46,
	MS_MegaVolts = 47, MS_MoonRay = 48, MS_NaturesCure = 49, MS_Pain = 50,
	MS_PoisonVolley = 51, MS_PowerCure = 52, MS_PowerShield = 53,
	MS_PrismaticLight = 54, MS_ProtFromElements = 55, MS_RaiseDead = 56,
	MS_RechargeItem = 57, MS_Resurrection = 58, MS_Revitalize = 59,
	MS_Shrapmetal = 60, MS_Sleep = 61, MS_Sparks = 62, MS_StarBurst = 63,
	MS_StoneToFlesh = 64, MS_SunRay = 65, MS_SuperShelter = 66,
	MS_SuppressDisease = 67, MS_SuppressPoison = 68, MS_Teleport = 69,
	MS_TimeDistortion = 70, MS_TownPortal = 71, MS_ToxicCloud = 72,
	MS_TurnUndead = 73, MS_WalkOnWater = 74, MS_WizardEye = 75,
	NO_SPELL = 76, TOTAL_SPELLS = 76
};

class Spells {
private:
	XeenEngine *_vm;

	void load();

	void executeSpell(MagicSpell spellId);

	/**
	 * Spell being cast failed
	 */
	void spellFailed();

	// Spell list
	void acidSpray();
	void awaken();
	void beastMaster();
	void bless();
	void clairvoyance();
	void coldRay();
	void createFood();
	void cureDisease();
	void cureParalysis();
	void curePoison();
	void cureWounds();
	void dancingSword();
	void dayOfProtection();
	void dayOfSorcery();
	void deadlySwarm();
	void detectMonster();
	void divineIntervention();
	void dragonSleep();
	void elementalStorm();
	void enchantItem();
	void energyBlast();
	void etherialize();
	void fantasticFreeze();
	void fieryFlail();
	void fingerOfDeath();
	void fireball();
	void firstAid();
	void flyingFist();
	void frostbite();
	void golemStopper();
	void heroism();
	void holyBonus();
	void holyWord();
	void hypnotize();
	void identifyMonster();
	void implosion();
	void incinerate();
	void inferno();
	void insectSpray();
	void itemToGold();
	void jump();
	void levitate();
	void light();
	void lightningBolt();
	void lloydsBeacon();
	void magicArrow();
	void massDistortion();
	void megaVolts();
	void moonRay();
	void naturesCure();
	void pain();
	void poisonVolley();
	void powerCure();
	void powerShield();
	void prismaticLight();
	void protectionFromElements();
	void raiseDead();
	void rechargeItem();
	void resurrection();
	void revitalize();
	void shrapMetal();
	void sleep();
	void sparks();
	void starBurst();
	void stoneToFlesh();
	void sunRay();
	void superShelter();
	void suppressDisease();
	void suppressPoison();
	void teleport();
	void timeDistortion();
	void townPortal();
	void toxicCloud();
	void turnUndead();
	void walkOnWater();
	void wizardEye();

	void frostbite2();
public:
	Common::StringArray _spellNames;
	int _lastCaster;
public:
	Spells(XeenEngine *vm);

	int calcSpellCost(int spellId, int expenseFactor) const;

	int calcSpellPoints(int spellId, int expenseFactor) const;

	/**
	 * Cast a spell associated with an item
	 */
	void castItemSpell(int itemSpellId);

	/**
	 * Cast a given spell
	 */
	int castSpell(Character *c, MagicSpell spellId);

	/**
	 * Subtract the requirements for a given spell if available, returning
	 * true if there was sufficient
	 */
	int subSpellCost(Character &c, int spellId);

	/**
	 * Add the SP and gem requirements for a given spell to the given
	 * character and party
	 */
	void addSpellCost(Character &c, int spellId);
};

} // End of namespace Xeen

#endif /* XEEN_SPELLS_H */
