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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "common/scummsys.h"
#include "cc.h"

enum MagicSpell {
	MS_AcidSpray = 0,
	MS_Awaken = 1,
	MS_BeastMaster = 2,
	MS_Bless = 3,
	MS_Clairvoyance = 4,
	MS_ColdRay = 5,
	MS_CreateFood = 6,
	MS_CureDisease = 7,
	MS_CureParalysis = 8,
	MS_CurePoison = 9,
	MS_CureWounds = 10,
	MS_DancingSword = 11,
	MS_DayOfProtection = 12,
	MS_DayOfSorcery = 13,
	MS_DeadlySwarm = 14,
	MS_DetectMonster = 15,
	MS_DivineIntervention = 16,
	MS_DragonSleep = 17,
	MS_ElementalStorm = 18,
	MS_EnchantItem = 19,
	MS_EnergyBlast = 20,
	MS_Etheralize = 21,
	MS_FantasticFreeze = 22,
	MS_FieryFlail = 23,
	MS_FingerOfDeath = 24,
	MS_Fireball = 25,
	MS_FirstAid = 26,
	MS_FlyingFist = 27,
	MS_FrostBite = 28,
	MS_GolemStopper = 29,
	MS_Heroism = 30,
	MS_HolyBonus = 31,
	MS_HolyWord = 32,
	MS_Hynotize = 33,
	MS_IdentifyMonster = 34,
	MS_Implosion = 35,
	MS_Incinerate = 36,
	MS_Inferno = 37,
	MS_InsectSpray = 38,
	MS_ItemToGold = 39,
	MS_Jump = 40,
	MS_Levitate = 41,
	MS_Light = 42,
	MS_LightningBolt = 43,
	MS_LloydsBeacon = 44,
	MS_MagicArrow = 45,
	MS_MassDistortion = 46,
	MS_MegaVolts = 47,
	MS_MoonRay = 48,
	MS_NaturesCure = 49,
	MS_Pain = 50,
	MS_PoisonVolley = 51,
	MS_PowerCure = 52,
	MS_PowerShield = 53,
	MS_PrismaticLight = 54,
	MS_ProtFromElements = 55,
	MS_RaiseDead = 56,
	MS_RechargeItem = 57,
	MS_Resurrection = 58,
	MS_Revitalize = 59,
	MS_Shrapmetal = 60,
	MS_Sleep = 61,
	MS_Sparks = 62,
	MS_StarBurst = 63,
	MS_StoneToFlesh = 64,
	MS_SunRay = 65,
	MS_SuperShelter = 66,
	MS_SuppressDisease = 67,
	MS_SuppressPoison = 68,
	MS_Teleport = 69,
	MS_TimeDistortion = 70,
	MS_TownPortal = 71,
	MS_ToxicCloud = 72,
	MS_TurnUndead = 73,
	MS_WalkOnWater = 74,
	MS_WizardEye = 75,
	NO_SPELL = 76
};

class LangConstants {
protected:
	const char **_gameNames = NULL;
	const char **_whoWillActions = NULL;
	const char **_whoActions = NULL;
	const char **_directionText = NULL;
	const char **_directionTextUpper = NULL;
	const char **_raceNames = NULL;
	const char **_sexNames = NULL;
	const char **_alignmentNames = NULL;
	const char **_skillNames = NULL;
	const char **_classNames = NULL;
	const char **_conditionNames = NULL;
	const char **_depositWithdrawl = NULL;
	const char **_statNames = NULL;
	const char **_whereNames = NULL;
	const char **_consumableNames = NULL;
	const char **_weekDayStrings = NULL;
	const char **_ratingText = NULL;
	const char **_bonusNames = NULL;
	const char **_weaponNames = NULL;
	const char **_armorNames = NULL;
	const char **_accessoryNames = NULL;
	const char **_miscNames = NULL;
	const char **_elementalNames = NULL;
	const char **_specialNames = NULL;
	const char **_attributeNames = NULL;
	const char **_effectivenessNames = NULL;
	const char **_questItemNamesSwords = NULL;
	const char **_questItemNames = NULL;
	const char **_categoryNames = NULL;
	const char **_itemActions = NULL;
	const char **_categoryBackpackIsFull = NULL;
	const char **_fixIdentify = NULL;
	const char **_removeDelete = NULL;
	const char **_spellCastComponents = NULL;
	const char **_monsterSpecialAttacks = NULL;
	const char **_quickFightOptions = NULL;
	const char **_goober = NULL;
	const char **_worldEndText = NULL;
	const char **_days = NULL;
	const char **_sellXForYGoldEndings = NULL;

public:
	virtual const char  *CLOUDS_CREDITS() = 0;
	virtual const char  *DARK_SIDE_CREDITS() = 0;
	virtual const char  *SWORDS_CREDITS1() = 0;
	virtual const char  *SWORDS_CREDITS2() = 0;
	virtual const char  *OPTIONS_MENU() = 0;
	virtual const char **GAME_NAMES() = 0;
	virtual const char  *THE_PARTY_NEEDS_REST() = 0;
	virtual const char  *WHO_WILL() = 0;
	virtual const char  *HOW_MUCH() = 0;
	virtual const char  *WHATS_THE_PASSWORD() = 0;
	virtual const char  *PASSWORD_INCORRECT() = 0;
	virtual const char  *IN_NO_CONDITION() = 0;
	virtual const char  *NOTHING_HERE() = 0;

	const char *const TERRAIN_TYPES[6] = {
		"town", "cave", "towr", "cstl", "dung", "scfi"
	};

	const char *const OUTDOORS_WALL_TYPES[16] = {
		nullptr, "mount", "ltree", "dtree", "grass", "snotree", "dsnotree",
		"snomnt", "dedltree", "mount", "lavamnt", "palm", "dmount", "dedltree",
		"dedltree", "dedltree"
	};

	const char *const SURFACE_NAMES[16] = {
		"water.srf", "dirt.srf", "grass.srf", "snow.srf", "swamp.srf",
		"lava.srf", "desert.srf", "road.srf", "dwater.srf", "tflr.srf",
		"sky.srf", "croad.srf", "sewer.srf", "cloud.srf", "scortch.srf",
		"space.srf"
	};

	virtual const char **WHO_ACTIONS() = 0;
	virtual const char **WHO_WILL_ACTIONS() = 0;

	const byte SYMBOLS[20][64] = {
		{ // 0
			0x00, 0x00, 0xA8, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x00, 0xA8, 0x9E, 0x9C, 0x9C, 0x9E, 0x9E, 0x9E,
			0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0x9A, 0x9A, 0x9A, 0xAC, 0x9E, 0xAC, 0xA8, 0xA8, 0xA6, 0x97, 0x98,
			0xAC, 0xA0, 0xAC, 0xAC, 0xA4, 0xA6, 0x98, 0x99, 0x00, 0xAC, 0xA0, 0xA0, 0xA8, 0xAC, 0x9A, 0x9A,
			0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xA4, 0x9B, 0x9A, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9B, 0x9B,
		},
		{ // 1
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
			0x99, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x99, 0x98, 0x98, 0x98, 0x97, 0x97, 0x97, 0x97, 0x97,
			0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
			0x9A, 0x9B, 0x9B, 0x9C, 0x9B, 0x9A, 0x9C, 0x9A, 0x9B, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x9A, 0x9B,
		},
		{ // 2
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
			0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x97, 0x98, 0x98,
			0x99, 0x98, 0x98, 0x98, 0x99, 0x99, 0x98, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
			0x9B, 0x9B, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9B, 0x99, 0x9A, 0x9B, 0x9B, 0x9A, 0x9A, 0x99, 0x9A,
		},
		{ // 3
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
			0x99, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x98, 0x98, 0x97, 0x97, 0x98, 0x98, 0x98, 0x98,
			0x99, 0x99, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
			0x9B, 0x9C, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x9A,
		},
		{ // 4
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
			0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x99, 0x9A, 0x97, 0x97, 0x97, 0x97, 0x97, 0x98, 0x98, 0x98,
			0x99, 0x99, 0x98, 0x99, 0x99, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
			0x9A, 0x9C, 0x9B, 0x9B, 0x9C, 0x9B, 0x9B, 0x9B, 0x9A, 0x99, 0x9B, 0x9B, 0x9A, 0x99, 0x9A, 0x9A,
		},
		{ // 5
			0xA4, 0xA4, 0xA8, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x9E, 0x9E, 0xA0, 0xA8, 0xAC, 0x00, 0x00,
			0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9E, 0xAC, 0x00, 0x97, 0x97, 0x97, 0x98, 0x9C, 0x9C, 0xA0, 0xAC,
			0x99, 0x98, 0x99, 0x99, 0x99, 0x9B, 0xA0, 0xAC, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9B, 0xA0, 0xAC,
			0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x9A, 0x9A, 0x9B, 0x9B, 0xA4, 0xAC, 0x00,
		},
		{ // 6
			0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9B, 0x99,
			0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x98, 0x99, 0x99,
			0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A, 0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x99, 0x99, 0x99,
			0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99,
		},
		{ // 7
			0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
			0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x9C, 0x99, 0x99,
			0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99,
			0x00, 0x00, 0xAC, 0xA0, 0x9B, 0xA0, 0x9E, 0x9C, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9C, 0x99, 0x99,
		},
		{ // 8
			0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9B, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
			0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
			0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
			0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9C, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9E, 0x9C, 0x99,
		},
		{ // 9
			0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x9C, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A,
			0xAC, 0xA4, 0x9C, 0x9A, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
			0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99,
			0x00, 0xAC, 0xA4, 0x9C, 0x9A, 0x9C, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9A, 0x99, 0x99,
		},
		{ // 10
			0x99, 0x99, 0x99, 0x9A, 0xA0, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
			0x99, 0x99, 0x9C, 0x9E, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9C, 0x99, 0x9C, 0xA4, 0xAC, 0x00,
			0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
			0x99, 0x99, 0x99, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9A, 0x9B, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
		},
		{ // 11
			0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0x9E, 0xAC,
			0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
			0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
			0x9C, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x9E, 0x9E, 0x9C, 0x9C, 0xA0, 0xAC, 0x00,
		},
		{ // 12
			0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9B, 0x9C, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
			0x99, 0x99, 0x99, 0x99, 0x99, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
			0x99, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00,
			0x99, 0x99, 0x9C, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		},
		{ // 13
			0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
			0x99, 0x9B, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9A, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
			0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
			0x99, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x00,
		},
		{ // 14
			0x00, 0x00, 0xAC, 0x9E, 0x9C, 0x9C, 0x9C, 0x9B, 0x00, 0xAC, 0x9C, 0xA0, 0x9E, 0xA4, 0xA4, 0xA4,
			0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0xAC, 0x9C, 0x9E, 0xAC, 0xA0, 0xAC, 0xA8, 0x9E, 0xA8, 0xAC, 0x99,
			0xAC, 0x9E, 0xAC, 0xA8, 0xAC, 0x9E, 0xA4, 0xAC, 0xAC, 0xA4, 0xA0, 0xAC, 0xAC, 0xA0, 0xA4, 0xAC,
			0x00, 0xAC, 0xA4, 0xA0, 0xA0, 0xA4, 0xAC, 0xA4, 0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
		},
		{ // 15
			0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
			0x9E, 0x9E, 0x9E, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x99, 0x99, 0x99, 0x99, 0x99, 0x98, 0x99, 0x98,
			0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E, 0x9E, 0xA0,
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
		},
		{ // 16
			0x9B, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0x9C, 0x9C, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
			0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x99, 0x99, 0x99, 0x99,
			0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0x9E, 0xA0, 0x9E, 0x9E, 0xA0,
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
		},
		{ // 17
			0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
			0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x9E, 0x98, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0x99, 0x98,
			0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9C, 0xA0, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E,
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
		},
		{ // 18
			0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
			0x9E, 0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x98, 0x99,
			0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0,
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
		},
		{ // 19
			0x9C, 0x9B, 0x9C, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0x00,
			0x9E, 0x9E, 0x9C, 0x9C, 0x9E, 0xA0, 0xAC, 0x00, 0x99, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0xA0, 0xAC,
			0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0xA0, 0xA0, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0xAC,
			0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0x00, 0x00,
		}
	};

	const byte TEXT_COLORS[40][4] = {
		{0x00, 0x19, 0x19, 0x19},
		{0x00, 0x08, 0x08, 0x08},
		{0x00, 0x0F, 0x0F, 0x0F},
		{0x00, 0x15, 0x15, 0x15},
		{0x00, 0x01, 0x01, 0x01},
		{0x00, 0x21, 0x21, 0x21},
		{0x00, 0x26, 0x26, 0x26},
		{0x00, 0x2B, 0x2B, 0x2B},
		{0x00, 0x31, 0x31, 0x31},
		{0x00, 0x36, 0x36, 0x36},
		{0x00, 0x3D, 0x3D, 0x3D},
		{0x00, 0x41, 0x41, 0x41},
		{0x00, 0x46, 0x46, 0x46},
		{0x00, 0x4C, 0x4C, 0x4C},
		{0x00, 0x50, 0x50, 0x50},
		{0x00, 0x55, 0x55, 0x55},
		{0x00, 0x5D, 0x5D, 0x5D},
		{0x00, 0x60, 0x60, 0x60},
		{0x00, 0x65, 0x65, 0x65},
		{0x00, 0x6C, 0x6C, 0x6C},
		{0x00, 0x70, 0x70, 0x70},
		{0x00, 0x75, 0x75, 0x75},
		{0x00, 0x7B, 0x7B, 0x7B},
		{0x00, 0x80, 0x80, 0x80},
		{0x00, 0x85, 0x85, 0x85},
		{0x00, 0x8D, 0x8D, 0x8D},
		{0x00, 0x90, 0x90, 0x90},
		{0x00, 0x97, 0x97, 0x97},
		{0x00, 0x9D, 0x9D, 0x9D},
		{0x00, 0xA4, 0xA4, 0xA4},
		{0x00, 0xAB, 0xAB, 0xAB},
		{0x00, 0xB0, 0xB0, 0xB0},
		{0x00, 0xB6, 0xB6, 0xB6},
		{0x00, 0xBD, 0xBD, 0xBD},
		{0x00, 0xC0, 0xC0, 0xC0},
		{0x00, 0xC6, 0xC6, 0xC6},
		{0x00, 0xCD, 0xCD, 0xCD},
		{0x00, 0xD0, 0xD0, 0xD0},
		{0x00, 0xD6, 0xD6, 0xD6},
		{0x00, 0xDB, 0xDB, 0xDB},
	};

	const byte TEXT_COLORS_STARTUP[40][4] = {
		{0x00, 0x19, 0x19, 0x19},
		{0x00, 0x08, 0x08, 0x08},
		{0x00, 0x0F, 0x0F, 0x0F},
		{0x00, 0x15, 0x15, 0x15},
		{0x00, 0x01, 0x01, 0x01},
		{0x00, 0x1F, 0x1F, 0x1F},
		{0x00, 0x26, 0x26, 0x26},
		{0x00, 0x2B, 0x2B, 0x2B},
		{0x00, 0x31, 0x31, 0x31},
		{0x00, 0x36, 0x36, 0x36},
		{0x00, 0x3D, 0x3D, 0x3D},
		{0x00, 0x42, 0x42, 0x42},
		{0x00, 0x46, 0x46, 0x46},
		{0x00, 0x4C, 0x4C, 0x4C},
		{0x00, 0x50, 0x50, 0x50},
		{0x00, 0x55, 0x55, 0x55},
		{0x00, 0x5D, 0x5D, 0x5D},
		{0x00, 0x60, 0x60, 0x60},
		{0x00, 0x65, 0x65, 0x65},
		{0x00, 0x6C, 0x6C, 0x6C},
		{0x00, 0x70, 0x70, 0x70},
		{0x00, 0x75, 0x75, 0x75},
		{0x00, 0x7B, 0x7B, 0x7B},
		{0x00, 0x80, 0x80, 0x80},
		{0x00, 0x85, 0x85, 0x85},
		{0x00, 0x8D, 0x8D, 0x8D},
		{0x00, 0x90, 0x90, 0x90},
		{0x00, 0x97, 0x97, 0x97},
		{0x00, 0x9D, 0x9D, 0x9D},
		{0x00, 0xA4, 0xA4, 0xA4},
		{0x00, 0xAB, 0xAB, 0xAB},
		{0x00, 0xB0, 0xB0, 0xB0},
		{0x00, 0xB6, 0xB6, 0xB6},
		{0x00, 0xBD, 0xBD, 0xBD},
		{0x00, 0xC0, 0xC0, 0xC0},
		{0x00, 0xC6, 0xC6, 0xC6},
		{0x00, 0xCD, 0xCD, 0xCD},
		{0x00, 0xD0, 0xD0, 0xD0},
		{0x00, 0x19, 0x19, 0x19},
		{0x00, 0x31, 0x31, 0x31}
	};

	virtual const char **DIRECTION_TEXT_UPPER() = 0;
	virtual const char **DIRECTION_TEXT() = 0;
	virtual const char **RACE_NAMES() = 0;

	const int RACE_HP_BONUSES[5] = {0, -2, 1, -1, 2};

	const int RACE_SP_BONUSES[5][2] = {
		{0, 0}, {2, 0}, {-1, -1}, {1, 1}, {-2, -2}
	};

	virtual const char **ALIGNMENT_NAMES() = 0;
	virtual const char **SEX_NAMES() = 0;
	virtual const char **SKILL_NAMES() = 0;
	virtual const char **CLASS_NAMES() = 0;

	const int CLASS_EXP_LEVELS[10] = {
		1500, 2000, 2000, 1500, 2000, 1000, 1500, 1500, 1500, 2000
	};

	virtual const char **CONDITION_NAMES() = 0;

	const int CONDITION_COLORS[17] = {
		9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
	};

	virtual const char *GOOD() = 0;
	virtual const char *BLESSED() = 0;
	virtual const char *POWER_SHIELD() = 0;
	virtual const char *HOLY_BONUS() = 0;
	virtual const char *HEROISM() = 0;
	virtual const char *IN_PARTY() = 0;
	virtual const char *PARTY_DETAILS() = 0;
	virtual const char *PARTY_DIALOG_TEXT() = 0;

	const int FACE_CONDITION_FRAMES[17] = {
		2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0
	};

	const int CHAR_FACES_X[6] = {10, 45, 81, 117, 153, 189};

	const int HP_BARS_X[6] = {13, 50, 86, 122, 158, 194};

	virtual const char *NO_ONE_TO_ADVENTURE_WITH() = 0;
	virtual const char *YOUR_ROSTER_IS_FULL() = 0;

	const byte DARKNESS_XLAT[3][256] = {
		{
		0, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}, {
		0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0,
		56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0,
		72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0,
		88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0,
		104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0,
		120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0,
		136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0,
		160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0,
		200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0,
		216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0,
		232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0,
		248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0
		}, {
		0, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
		36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0,
		68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0,
		84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0,
		100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0,
		116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0,
		132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0,
		152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
		168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
		180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0,
		196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0,
		212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0,
		228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0,
		244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0
		}
	};

	virtual const char *PLEASE_WAIT() = 0;
	virtual const char *OOPS() = 0;

	const int SCREEN_POSITIONING_X[4][48] = {
		{
		-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
		-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
		-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
		-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
		}, {
		 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
		 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
		 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
		 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
		}, {
		 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
		 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
		 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
		 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
		}, {
		 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
		-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
		-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
		-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
		}
	};

	const int SCREEN_POSITIONING_Y[4][48] = {
		{
		 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
		 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
		 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
		 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
		}, {
		 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
		 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
		 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
		 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
		}, {
		 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
		-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
		-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
		-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
		}, {
		-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
		-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
		-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
		-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
		}
	};

	const int MONSTER_GRID_BITMASK[12] = {
		0xC, 8, 4, 0, 0xF, 0xF000, 0xF00, 0xF0, 0xF00, 0xF0, 0x0F, 0xF000
	};

	const int INDOOR_OBJECT_X[2][12] = {
		{ -5, -7, -112, 98, -8, -65, 49, -9, -34, 16, -58, 40 },
		{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -14, -98, 16 }
	};

	const int MAP_OBJECT_Y[2][12] = {
		{ 2, 25, 25, 25, 50, 50, 50, 58, 58, 58, 58, 58 },
		{ -65, -6, -6, -6, 36, 36, 36, 54, 54, 54, 54, 54 }
	};

	const int INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

	const int OUTDOOR_OBJECT_X[2][12] = {
		{ -5, -7, -112, 98, -8, -77, 61, -9, -43, 25, -74, 56 },
		{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -24, -98, 16 }
	};

	const int OUTDOOR_MONSTER_INDEXES[26] = {
		42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 69, 70,
		71, 72, 73, 74, 75, 90, 91, 92, 93, 94, 112, 115, 118
	};

	const int OUTDOOR_MONSTERS_Y[26] = {
		59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53, 53,
		53, 53, 53, 53, 53, 34, 34, 34, 34, 34, 2, 2, 2
	};

	const int DIRECTION_ANIM_POSITIONS[4][4] = {
		{ 0, 1, 2, 3 }, { 3, 0, 1, 2 }, { 2, 3, 0, 1 }, { 1, 2, 3, 0 }
	};

	const byte WALL_SHIFTS[4][48] = {
		{
			12, 0, 12, 8, 12, 12, 0, 12, 8, 12, 12, 0,
			12, 0, 12, 8, 12, 8, 12, 12, 0, 12, 0, 12,
			0, 12, 0, 12, 8, 12, 8, 12, 8, 12, 8, 12,
			0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 4, 4
		}, {
			8, 12, 8, 4, 8, 8, 12, 8, 4, 8, 8, 12,
			8, 12, 8, 4, 8, 4, 8, 8, 12, 8, 12, 8,
			12, 8, 12, 8, 4, 8, 4, 8, 4, 8, 4, 8,
			12, 12, 12, 12, 4, 4, 4, 4, 0, 0, 0, 0
		}, {
			4, 8, 4, 0, 4, 4, 8, 4, 0, 4, 4, 8,
			4, 8, 4, 0, 4, 0, 4, 4, 8, 4, 8, 4,
			8, 4, 8, 4, 0, 4, 0, 4, 0, 4, 0, 4,
			8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 12, 12
		}, {
			0, 4, 0, 12, 0, 0, 4, 0, 12, 0, 0, 4,
			0, 4, 0, 12, 0, 12, 0, 0, 4, 0, 4, 0,
			4, 0, 4, 0, 12, 0, 12, 0, 12, 0, 12, 0,
			4, 4, 4, 4, 12, 12, 12, 12, 0, 0, 8, 8
		}
	};

	const int DRAW_NUMBERS[25] = {
		36, 37, 38, 43, 42, 41,
		39, 20, 22, 24, 33, 31,
		29, 26, 10, 11, 18, 16,
		13, 5, 9, 6, 0, 4, 1
	};

	const int DRAW_FRAMES[25][2] = {
		{ 18, 24 }, { 19, 23 }, { 20, 22 }, { 24, 18 }, { 23, 19 }, { 22, 20 },
		{ 21, 21 }, { 11, 17 }, { 12, 16 }, { 13, 15 }, { 17, 11 }, { 16, 12 },
		{ 15, 13 }, { 14, 14 }, { 6, 10 }, { 7, 9 }, { 10, 6 }, { 9, 7 },
		{ 8, 8 }, { 3, 5 }, { 5, 3 }, { 4, 4 }, { 0, 2 }, { 2, 0 },
		{ 1, 1 }
	};

	const int COMBAT_FLOAT_X[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };

	const int COMBAT_FLOAT_Y[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

	const int MONSTER_EFFECT_FLAGS[15][8] = {
		{ 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B },
		{ 0x10C, 0x10D, 0x10E, 0x10F, 0x0, 0x0, 0x0, 0x0 },
		{ 0x110, 0x111, 0x112, 0x113, 0x0, 0x0, 0x0, 0x0 },
		{ 0x114, 0x115, 0x116, 0x117, 0x0, 0x0, 0x0, 0x0 },
		{ 0x200, 0x201, 0x202, 0x203, 0x0, 0x0, 0x0, 0x0 },
		{ 0x300, 0x301, 0x302, 0x303, 0x400, 0x401, 0x402, 0x403 },
		{ 0x500, 0x501, 0x502, 0x503, 0x0, 0x0, 0x0, 0x0 },
		{ 0x600, 0x601, 0x602, 0x603, 0x0, 0x0, 0x0, 0x0 },
		{ 0x604, 0x605, 0x606, 0x607, 0x608, 0x609, 0x60A, 0x60B },
		{ 0x60C, 0x60D, 0x60E, 0x60F, 0x0, 0x0, 0x0, 0x0 },
		{ 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100 },
		{ 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101 },
		{ 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102 },
		{ 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103 },
		{ 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108 }
	};

	const int SPELLS_ALLOWED[3][40] = {
		{
			0, 1, 2, 3, 5, 6, 7, 8, 9, 10,
			12, 14, 16, 23, 26, 27, 28, 30, 31, 32,
			33, 42, 46, 48, 49, 50, 52, 55, 56, 58,
			59, 62, 64, 65, 67, 68, 71, 73, 74, 76
		}, {
			1, 4, 11, 13, 15, 17, 18, 19, 20, 21,
			22, 24, 25, 29, 34, 35, 36, 37, 38, 39,
			40, 41, 42, 43, 44, 45, 47, 51, 53, 54,
			57, 60, 61, 63, 66, 69, 70, 72, 75, 76
		}, {
			0, 1, 2, 3, 4, 5, 7, 9, 10, 20,
			25, 26, 27, 28, 30, 31, 34, 38, 40, 41,
			42, 43, 44, 45, 49, 50, 52, 53, 55, 59,
			60, 61, 62, 67, 68, 72, 73, 74, 75, 76
		}
	};

	const int BASE_HP_BY_CLASS[10] = { 10, 8, 7, 5, 4, 8, 7, 12, 6, 9 };

	const int AGE_RANGES[10] = { 1, 6, 11, 18, 36, 51, 76, 101, 201, 0xffff };

	const int AGE_RANGES_ADJUST[2][10] = {
		{ -250, -50, -20, -10, 0, -2, -5, -10, -20, -50 },
		{ -250, -50, -20, -10, 0, 2, 5, 10, 20, 50 }
	};

	const int STAT_VALUES[24] = {
		3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 30, 35, 40,
		50, 75, 100, 125, 150, 175, 200, 225, 250, 65535
	};

	const int STAT_BONUSES[24] = {
		-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
		7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20
	};

	const int ELEMENTAL_CATEGORIES[6] = { 8, 15, 20, 25, 33, 36 };

	const int ATTRIBUTE_CATEGORIES[10] = {
		9, 17, 25, 33, 39, 45, 50, 56, 61, 72
	};

	const int ATTRIBUTE_BONUSES[72] = {
		2, 3, 5, 8, 12, 17, 23, 30, 38, 47,	// Might bonus
		2, 3, 5, 8, 12, 17, 23, 30,			// INT bonus
		2, 3, 5, 8, 12, 17, 23, 30,			// PER bonus
		2, 3, 5, 8, 12, 17, 23, 30,			// SPD bonus
		3, 5, 10, 15, 20, 30,				// ACC bonus
		5, 10, 15, 20, 25, 30,				// LUC bonus
		4, 6, 10, 20, 50,					// HP bonus
		4, 8, 12, 16, 20, 25,				// SP bonus
		2, 4, 6, 10, 16,					// AC bonus
		4, 6, 8, 10, 12, 14, 16, 18, 20, 25	// Thievery bonus
	};

	const int ELEMENTAL_RESISTENCES[37] = {
		0, 5, 7, 9, 12, 15, 20, 25, 30, 5, 7, 9, 12, 15, 20, 25,
		5, 10, 15, 20, 25, 10, 15, 20, 25, 40, 5, 7, 9, 11, 13, 15, 20, 25,
		5, 10, 20
	};

	const int ELEMENTAL_DAMAGE[37] = {
		0, 2, 3, 4, 5, 10, 15, 20, 30, 2, 3, 4, 5, 10, 15, 20, 2, 4, 5, 10, 20,
		2, 4, 8, 16, 32, 2, 3, 4, 5, 10, 15, 20, 30, 5, 10, 25
	};

	const int WEAPON_DAMAGE_BASE[35] = {
		0, 3, 2, 3, 2, 2, 4, 1, 2, 4, 2, 3,
		2, 2, 1, 1, 1, 1, 4, 4, 3, 2, 4, 2,
		2, 2, 5, 3, 3, 3, 3, 5, 4, 2, 6
	};

	const int WEAPON_DAMAGE_MULTIPLIER[35] = {
		0, 3, 3, 4, 5, 4, 2, 3, 3, 3, 3, 3,
		2, 4, 10, 6, 8, 9, 4, 3, 6, 8, 5, 6,
		4, 5, 3, 5, 6, 7, 2, 2, 2, 2, 4
	};

	const int METAL_DAMAGE[22] = {
		-3, -6, -4, -2, 2, 4, 6, 8, 10, 0, 1,
		1, 2, 2, 3, 4, 5, 12, 15, 20, 30, 50
	};

	const int METAL_DAMAGE_PERCENT[22] = {
		253, 252, 3, 2, 1, 2, 3, 4, 6, 0, 1,
		1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10
	};

	const int METAL_LAC[22] = {
		-3, 0, -2, -1, 1, 2, 4, 6, 8, 0, 1,
		1, 2, 2, 3, 4, 5, 10, 12, 14, 16, 20
	};

	const int ARMOR_STRENGTHS[14] = { 0, 2, 4, 5, 6, 7, 8, 10, 4, 2, 1, 1, 1, 1 };

	const int MAKE_ITEM_ARR1[6] = { 0, 8, 15, 20, 25, 33 };

	const int MAKE_ITEM_ARR2[6][7][2] = {
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 7 }, { 7, 7 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 5, 5 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
		{ { 0, 0 }, { 1, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 3 }, { 3, 3 } }
	};

	const int MAKE_ITEM_ARR3[10][7][2] = {
		{ { 0, 0 }, { 1, 4 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 10 }, { 10, 10 } },
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
		{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
		{ { 0, 0 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 6 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
		{ { 0, 0 }, { 1, 2 }, { 1, 4 }, { 3, 6 }, { 5, 8 }, { 7, 10 }, { 10, 10 } }
	};

	const int MAKE_ITEM_ARR4[2][7][2] = {
		{ { 0, 0 }, { 1, 4 }, { 3, 7 }, { 4, 8 }, { 5, 9 }, { 8, 9 }, { 9, 9 } },
		{ { 0, 0 }, { 1, 4 }, { 2, 6 }, { 4, 7 }, { 6, 10 }, { 9, 13 }, { 13, 13 } }
	};

	const int MAKE_ITEM_ARR5[8][2] = {
		{ 0, 0 }, { 1, 15 }, { 16, 30 }, { 31, 40 }, { 41, 50 },
		{ 51, 60 }, { 61, 73 }, { 61, 73 }
	};

	const int OUTDOOR_DRAWSTRUCT_INDEXES[44] = {
		37, 38, 39, 40, 41, 44, 42, 43, 47, 45, 46,
		48, 49, 52, 50, 51, 66, 67, 68, 69, 70, 71,
		72, 75, 73, 74, 87, 88, 89, 90, 91, 94, 92,
		93, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120
	};

	const int TOWN_MAXES[2][11] = {
		{ 23, 13, 32, 16, 26, 16, 16, 16, 16, 16, 16 },
		{ 26, 19, 48, 27, 26, 37, 16, 16, 16, 16, 16 }
	};

	const char *const TOWN_ACTION_MUSIC[2][7] = {
		{ "bank.m", "smith.m", "guild.m", "tavern.m",
		"temple.m", "grounds.m", "endgame.m" },
		{ "bank.m", "sf09.m", "guild.m", "tavern.m",
		"temple.m", "smith.m", "endgame.m" }
	};

	const char *const TOWN_ACTION_SHAPES[7] = {
		"bnkr", "blck", "gild", "tvrn", "tmpl", "trng", "eface08"
	};

	const int TOWN_ACTION_FILES[2][7] = {
		{ 3, 2, 4, 2, 4, 2, 1 }, { 5, 3, 7, 5, 4, 6, 1 }
	};

	virtual const char *BANK_TEXT() = 0;
	virtual const char *BLACKSMITH_TEXT() = 0;
	virtual const char *GUILD_NOT_MEMBER_TEXT() = 0;
	virtual const char *GUILD_TEXT() = 0;
	virtual const char *TAVERN_TEXT() = 0;
	virtual const char *FOOD_AND_DRINK() = 0;
	virtual const char *GOOD_STUFF() = 0;
	virtual const char *HAVE_A_DRINK() = 0;
	virtual const char *YOURE_DRUNK() = 0;

	const int TAVERN_EXIT_LIST[2][6][5][2] = {
		{
			{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
			{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
			{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
			{ { 18, 4 }, { 0, 0 }, { 19, 16 }, { 0, 0 }, { 11, 12 } },
			{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
			{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } },
		}, {
			{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
			{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
			{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
			{ { 17, 24 }, { 14, 13 }, { 0, 0 }, { 0, 0 }, { 9, 4 } },
			{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
			{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } }
		}
	};

	virtual const char  *TEMPLE_TEXT() = 0;
	virtual const char  *EXPERIENCE_FOR_LEVEL() = 0;
	virtual const char  *TRAINING_LEARNED_ALL() = 0;
	virtual const char  *ELIGIBLE_FOR_LEVEL() = 0;
	virtual const char  *TRAINING_TEXT() = 0;
	virtual const char  *GOLD_GEMS() = 0;
	virtual const char  *GOLD_GEMS_2() = 0;
	virtual const char **DEPOSIT_WITHDRAWL() = 0;
	virtual const char  *NOT_ENOUGH_X_IN_THE_Y() = 0;
	virtual const char  *NO_X_IN_THE_Y() = 0;
	virtual const char **STAT_NAMES() = 0;
	virtual const char **CONSUMABLE_NAMES() = 0;
	virtual const char **WHERE_NAMES() = 0;
	virtual const char  *AMOUNT() = 0;
	virtual const char  *FOOD_PACKS_FULL() = 0;
	virtual const char  *BUY_SPELLS() = 0;
	virtual const char  *GUILD_OPTIONS() = 0;

	const int MISC_SPELL_INDEX[74] = {
		NO_SPELL, MS_Light, MS_Awaken, MS_MagicArrow,
		MS_FirstAid, MS_FlyingFist, MS_EnergyBlast, MS_Sleep,
		MS_Revitalize, MS_CureWounds, MS_Sparks, MS_Shrapmetal,
		MS_InsectSpray, MS_ToxicCloud, MS_ProtFromElements, MS_Pain,
		MS_Jump, MS_BeastMaster, MS_Clairvoyance, MS_TurnUndead,
		MS_Levitate, MS_WizardEye, MS_Bless, MS_IdentifyMonster,
		MS_LightningBolt, MS_HolyBonus, MS_PowerCure, MS_NaturesCure,
		MS_LloydsBeacon, MS_PowerShield, MS_Heroism, MS_Hynotize,
		MS_WalkOnWater, MS_FrostBite, MS_DetectMonster, MS_Fireball,
		MS_ColdRay, MS_CurePoison, MS_AcidSpray, MS_TimeDistortion,
		MS_DragonSleep, MS_CureDisease, MS_Teleport, MS_FingerOfDeath,
		MS_CureParalysis, MS_GolemStopper, MS_PoisonVolley, MS_DeadlySwarm,
		MS_SuperShelter, MS_DayOfProtection, MS_DayOfSorcery, MS_CreateFood,
		MS_FieryFlail, MS_RechargeItem, MS_FantasticFreeze, MS_TownPortal,
		MS_StoneToFlesh, MS_RaiseDead, MS_Etheralize, MS_DancingSword,
		MS_MoonRay, MS_MassDistortion, MS_PrismaticLight, MS_EnchantItem,
		MS_Incinerate, MS_HolyWord, MS_Resurrection, MS_ElementalStorm,
		MS_MegaVolts, MS_Inferno, MS_SunRay, MS_Implosion,
		MS_StarBurst, MS_DivineIntervention
	};

	const int SPELL_COSTS[77] = {
		8, 1, 5, -2, 5, -2, 20, 10, 12, 8, 3,
		- 3, 75, 40, 12, 6, 200, 10, 100, 30, -1, 30,
		15, 25, 10, -2, 1, 2, 7, 20, -2, -2, 100,
		15, 5, 100, 35, 75, 5, 20, 4, 5, 1, -2,
		6, 2, 75, 40, 60, 6, 4, 25, -2, -2, 60,
		- 1, 50, 15, 125, 2, -1, 3, -1, 200, 35, 150,
		15, 5, 4, 10, 8, 30, 4, 5, 7, 5, 0
	};

	const int DARK_SPELL_RANGES[12][2] = {
		{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 },
		{ 0, 17 }, { 14, 34 }, { 26, 37 }, { 29, 39 },
		{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }
	};

	const int SWORDS_SPELL_RANGES[12][2] = {
		{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 },
		{ 0, 17 },{ 14, 34 },{ 26, 39 },{ 29, 39 },
		{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 }
	};

	const int CLOUDS_GUILD_SPELLS[5][20] = {
		{
			1, 10, 20, 26, 27, 38, 40, 42, 45, 50,
			55, 59, 60, 61, 62, 68, 72, 75, 77, 77
		}, {
			3, 4, 5, 14, 15, 25, 30, 31, 34, 41,
			49, 51, 53, 67, 73, 75, -1, -1, -1, -1
		}, {
			4, 8, 9, 12, 13, 22, 23, 24, 28, 34,
			41, 44, 52, 70, 73, 74, -1, -1, -1, -1
		}, {
			6, 7, 9, 11, 12, 13, 17, 21, 22, 24,
			29, 36, 56, 58, 64, 71, -1, -1, -1, -1
		}, {
			6, 7, 9, 11, 12, 13, 18, 21, 29, 32,
			36, 37, 46, 51, 56, 58, 69, -1, -1, -1
		}
	};

	const int DARK_SPELL_OFFSETS[3][39] = {
		{
			42, 1, 26, 59, 27, 10, 50, 68, 55, 62, 67, 73, 2,
			5, 3, 31, 30, 52, 49, 28, 74, 0, 9, 7, 14, 8,
			33, 6, 23, 71, 64, 56, 48, 46, 12, 32, 58, 65, 16
		}, {
			42, 1, 45, 61, 72, 40, 20, 60, 38, 41, 75, 34, 4,
			43, 25, 53, 44, 15, 70, 17, 24, 69, 22, 66, 57, 11,
			29, 39, 51, 21, 19, 36, 47, 13, 54, 37, 18, 35, 63
		}, {
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38
		}
	};

	const int SPELL_GEM_COST[77] = {
		0, 0, 2, 1, 2, 4, 5, 0, 0, 0, 0, 10, 10, 10, 0, 0, 20, 4, 10, 20, 1, 10,
		5, 5, 4, 2, 0, 0, 0, 10, 3, 1, 20, 4, 0, 20, 10, 10, 1, 10, 0, 0, 0, 2,
		2, 0, 10, 10, 10, 0, 0, 10, 3, 2, 10, 1, 10, 10, 20, 0, 0, 1, 1, 20, 5, 20,
		5, 0, 0, 0, 0, 5, 1, 2, 0, 2, 0
	};

	virtual const char  *NOT_A_SPELL_CASTER() = 0;
	virtual const char  *SPELLS_LEARNED_ALL() = 0;
	virtual const char  *SPELLS_FOR() = 0;
	virtual const char  *SPELL_LINES_0_TO_9() = 0;
	virtual const char  *SPELLS_DIALOG_SPELLS() = 0;
	virtual const char  *SPELL_PTS() = 0;
	virtual const char  *GOLD() = 0;
	virtual const char  *SPELL_INFO() = 0;
	virtual const char  *SPELL_PURCHASE() = 0;
	virtual const char  *MAP_TEXT() = 0;
	virtual const char  *LIGHT_COUNT_TEXT() = 0;
	virtual const char  *FIRE_RESISTENCE_TEXT() = 0;
	virtual const char  *ELECRICITY_RESISTENCE_TEXT() = 0;
	virtual const char  *COLD_RESISTENCE_TEXT() = 0;
	virtual const char  *POISON_RESISTENCE_TEXT() = 0;
	virtual const char  *CLAIRVOYANCE_TEXT() = 0;
	virtual const char  *LEVITATE_TEXT() = 0;
	virtual const char  *WALK_ON_WATER_TEXT() = 0;
	virtual const char  *GAME_INFORMATION() = 0;
	virtual const char  *WORLD_GAME_TEXT() = 0;
	virtual const char  *DARKSIDE_GAME_TEXT() = 0;
	virtual const char  *CLOUDS_GAME_TEXT() = 0;
	virtual const char  *SWORDS_GAME_TEXT() = 0;
	virtual const char **WEEK_DAY_STRINGS() = 0;
	virtual const char  *CHARACTER_DETAILS() = 0;
	virtual const char **DAYS() = 0;
	virtual const char  *PARTY_GOLD() = 0;
	virtual const char  *PLUS_14() = 0;
	virtual const char  *CHARACTER_TEMPLATE() = 0;
	virtual const char  *EXCHANGING_IN_COMBAT() = 0;
	virtual const char  *CURRENT_MAXIMUM_RATING_TEXT() = 0;
	virtual const char  *CURRENT_MAXIMUM_TEXT() = 0;
	virtual const char **RATING_TEXT() = 0;
	virtual const char  *AGE_TEXT() = 0;
	virtual const char  *LEVEL_TEXT() = 0;
	virtual const char  *RESISTENCES_TEXT() = 0;
	virtual const char  *NONE() = 0;
	virtual const char  *EXPERIENCE_TEXT() = 0;
	virtual const char  *ELIGIBLE() = 0;
	virtual const char  *IN_PARTY_IN_BANK() = 0;
	virtual const char  *FOOD_TEXT() = 0;
	virtual const char  *EXCHANGE_WITH_WHOM() = 0;
	virtual const char  *QUICK_REF_LINE() = 0;
	virtual const char  *QUICK_REFERENCE() = 0;

	const int BLACKSMITH_MAP_IDS[2][4] = { { 28, 30, 73, 49 }, { 29, 31, 37, 43 } };

	virtual const char  *ITEMS_DIALOG_TEXT1() = 0;
	virtual const char  *ITEMS_DIALOG_TEXT2() = 0;
	virtual const char  *ITEMS_DIALOG_LINE1() = 0;
	virtual const char  *ITEMS_DIALOG_LINE2() = 0;
	virtual const char  *BTN_BUY() = 0;
	virtual const char  *BTN_SELL() = 0;
	virtual const char  *BTN_IDENTIFY() = 0;
	virtual const char  *BTN_FIX() = 0;
	virtual const char  *BTN_USE() = 0;
	virtual const char  *BTN_EQUIP() = 0;
	virtual const char  *BTN_REMOVE() = 0;
	virtual const char  *BTN_DISCARD() = 0;
	virtual const char  *BTN_QUEST() = 0;
	virtual const char  *BTN_ENCHANT() = 0;
	virtual const char  *BTN_RECHARGE() = 0;
	virtual const char  *BTN_GOLD() = 0;
	virtual const char  *ITEM_BROKEN() = 0;
	virtual const char  *ITEM_CURSED() = 0;
	virtual const char  *ITEM_OF() = 0;
	virtual const char **BONUS_NAMES() = 0;
	virtual const char **WEAPON_NAMES() = 0;
	virtual const char **ARMOR_NAMES() = 0;
	virtual const char **ACCESSORY_NAMES() = 0;
	virtual const char **MISC_NAMES() = 0;
	virtual const char **SPECIAL_NAMES() = 0;
	virtual const char **ELEMENTAL_NAMES() = 0;
	virtual const char **ATTRIBUTE_NAMES() = 0;
	virtual const char **EFFECTIVENESS_NAMES() = 0;
	virtual const char **QUEST_ITEM_NAMES() = 0;
	virtual const char **QUEST_ITEM_NAMES_SWORDS() = 0;

	const int WEAPON_BASE_COSTS[35] = {
		0, 50, 15, 100, 80, 40, 60, 1, 10, 150, 30, 60, 8, 50,
		100, 15, 30, 15, 200, 80, 250, 150, 400, 100, 40, 120,
		300, 100, 200, 300, 25, 100, 50, 15, 0
	};
	const int ARMOR_BASE_COSTS[14] = {
		0, 20, 100, 200, 400, 600, 1000, 2000, 100, 60, 40, 250, 200, 100
	};
	const int ACCESSORY_BASE_COSTS[11] = {
		0, 100, 100, 250, 100, 50, 300, 200, 500, 1000, 2000
	};
	const int MISC_MATERIAL_COSTS[22] = {
		0, 50, 1000, 500, 10, 100, 20, 10, 50, 10, 10, 100,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	const int MISC_BASE_COSTS[76] = {
		0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
		100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200, 200,
		200, 200, 200, 200, 200, 200, 200, 300, 300, 300, 300, 300,
		300, 300, 300, 300, 300, 400, 400, 400, 400, 400, 400, 400,
		400, 400, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500,
		500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
		600, 600, 600, 600
	};
	const int METAL_BASE_MULTIPLIERS[22] = {
		10, 25, 5, 75, 2, 5, 10, 20, 50, 2, 3, 5, 10, 20, 30, 40,
		50, 60, 70, 80, 90, 100
	};
	const int ITEM_SKILL_DIVISORS[4] = { 1, 2, 100, 10 };

	const int RESTRICTION_OFFSETS[4] = { 0, 35, 49, 60 };

	const int ITEM_RESTRICTIONS[86] = {
		0, 86, 86, 86, 86, 86, 86, 0, 6, 239, 239, 239, 2, 4, 4, 4, 4,
		6, 70, 70, 70, 70, 94, 70, 0, 4, 239, 86, 86, 86, 70, 70, 70, 70,
		0, 0, 0, 68, 100, 116, 125, 255, 255, 85, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	virtual const char  *NOT_PROFICIENT() = 0;
	virtual const char  *NO_ITEMS_AVAILABLE() = 0;
	virtual const char **CATEGORY_NAMES() = 0;
	virtual const char  *X_FOR_THE_Y() = 0;
	virtual const char  *X_FOR_Y() = 0;
	virtual const char  *X_FOR_Y_GOLD() = 0;
	virtual const char  *FMT_CHARGES() = 0;
	virtual const char  *AVAILABLE_GOLD_COST() = 0;
	virtual const char  *CHARGES() = 0;
	virtual const char  *COST() = 0;
	virtual const char **ITEM_ACTIONS() = 0;
	virtual const char  *WHICH_ITEM() = 0;
	virtual const char  *WHATS_YOUR_HURRY() = 0;
	virtual const char  *USE_ITEM_IN_COMBAT() = 0;
	virtual const char  *NO_SPECIAL_ABILITIES() = 0;
	virtual const char  *CANT_CAST_WHILE_ENGAGED() = 0;
	virtual const char  *EQUIPPED_ALL_YOU_CAN() = 0;
	virtual const char  *REMOVE_X_TO_EQUIP_Y() = 0;
	virtual const char  *RING() = 0;
	virtual const char  *MEDAL() = 0;
	virtual const char  *CANNOT_REMOVE_CURSED_ITEM() = 0;
	virtual const char  *CANNOT_DISCARD_CURSED_ITEM() = 0;
	virtual const char  *PERMANENTLY_DISCARD() = 0;
	virtual const char  *BACKPACK_IS_FULL() = 0;
	virtual const char **CATEGORY_BACKPACK_IS_FULL() = 0;
	virtual const char  *BUY_X_FOR_Y_GOLD() = 0;
	virtual const char  *SELL_X_FOR_Y_GOLD() = 0;
	virtual const char **SELL_X_FOR_Y_GOLD_ENDINGS() = 0;
	virtual const char  *NO_NEED_OF_THIS() = 0;
	virtual const char  *NOT_RECHARGABLE() = 0;
	virtual const char  *NOT_ENCHANTABLE() = 0;
	virtual const char  *SPELL_FAILED() = 0;
	virtual const char  *ITEM_NOT_BROKEN() = 0;
	virtual const char **FIX_IDENTIFY() = 0;
	virtual const char  *FIX_IDENTIFY_GOLD() = 0;
	virtual const char  *IDENTIFY_ITEM_MSG() = 0;
	virtual const char  *ITEM_DETAILS() = 0;
	virtual const char  *ALL() = 0;
	virtual const char  *FIELD_NONE() = 0;
	virtual const char  *DAMAGE_X_TO_Y() = 0;
	virtual const char  *ELEMENTAL_XY_DAMAGE() = 0;
	virtual const char  *ATTR_XY_BONUS() = 0;
	virtual const char  *EFFECTIVE_AGAINST() = 0;
	virtual const char  *QUESTS_DIALOG_TEXT() = 0;
	virtual const char  *CLOUDS_OF_XEEN_LINE() = 0;
	virtual const char  *DARKSIDE_OF_XEEN_LINE() = 0;
	virtual const char  *SWORDS_OF_XEEN_LINE() = 0;
	virtual const char  *NO_QUEST_ITEMS() = 0;
	virtual const char  *NO_CURRENT_QUESTS() = 0;
	virtual const char  *NO_AUTO_NOTES() = 0;
	virtual const char  *QUEST_ITEMS_DATA() = 0;
	virtual const char  *CURRENT_QUESTS_DATA() = 0;
	virtual const char  *AUTO_NOTES_DATA() = 0;
	virtual const char  *REST_COMPLETE() = 0;
	virtual const char  *PARTY_IS_STARVING() = 0;
	virtual const char  *HIT_SPELL_POINTS_RESTORED() = 0;
	virtual const char  *TOO_DANGEROUS_TO_REST() = 0;
	virtual const char  *SOME_CHARS_MAY_DIE() = 0;
	virtual const char  *DISMISS_WHOM() = 0;
	virtual const char  *CANT_DISMISS_LAST_CHAR() = 0;
	virtual const char  *DELETE_CHAR_WITH_ELDER_WEAPON() = 0;
	virtual const char **REMOVE_DELETE() = 0;
	virtual const char  *REMOVE_OR_DELETE_WHICH() = 0;
	virtual const char  *YOUR_PARTY_IS_FULL() = 0;
	virtual const char  *HAS_SLAYER_SWORD() = 0;
	virtual const char  *SURE_TO_DELETE_CHAR() = 0;
	virtual const char  *CREATE_CHAR_DETAILS() = 0;
	virtual const char  *NEW_CHAR_STATS() = 0;
	virtual const char  *NAME_FOR_NEW_CHARACTER() = 0;
	virtual const char  *SELECT_CLASS_BEFORE_SAVING() = 0;
	virtual const char  *EXCHANGE_ATTR_WITH() = 0;

	const int NEW_CHAR_SKILLS[10] = { 1, 5, -1, -1, 4, 0, 0, -1, 6, 11 };
	const int NEW_CHAR_SKILLS_OFFSET[10] = { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 };
	const int NEW_CHAR_SKILLS_LEN[10] = { 11, 8, 0, 0, 12, 8, 8, 0, 9, 11 };
	const int NEW_CHAR_RACE_SKILLS[10] = { 14, -1, 17, 16, -1, 0, 0, 0, 0, 0 };

	const int RACE_MAGIC_RESISTENCES[5] = { 7, 5, 20, 0, 0 };
	const int RACE_FIRE_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
	const int RACE_ELECTRIC_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
	const int RACE_COLD_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
	const int RACE_ENERGY_RESISTENCES[5] = { 7, 5, 2, 5, 0 };
	const int RACE_POISON_RESISTENCES[5] = { 7, 0, 2, 20, 0 };
	const int NEW_CHARACTER_SPELLS[10][4] = {
		{ -1, -1, -1, -1 },
		{ 21, -1, -1, -1 },
		{ 22, -1, -1, -1 },
		{ 21, 1, 14, -1 },
		{ 22, 0, 25, -1 },
		{ -1, -1, -1, -1 },
		{ -1, -1, -1, -1 },
		{ -1, -1, -1, -1 },
		{ 20, 1, 11, 23 },
		{ 20, 1, -1, -1 }
	};

	virtual const char  *COMBAT_DETAILS() = 0;
	virtual const char  *NOT_ENOUGH_TO_CAST() = 0;
	virtual const char **SPELL_CAST_COMPONENTS() = 0;
	virtual const char  *CAST_SPELL_DETAILS() = 0;
	virtual const char  *PARTY_FOUND() = 0;
	virtual const char  *BACKPACKS_FULL_PRESS_KEY() = 0;
	virtual const char  *HIT_A_KEY() = 0;
	virtual const char  *GIVE_TREASURE_FORMATTING() = 0;
	virtual const char  *X_FOUND_Y() = 0;
	virtual const char  *ON_WHO() = 0;
	virtual const char  *WHICH_ELEMENT1() = 0;
	virtual const char  *WHICH_ELEMENT2() = 0;
	virtual const char  *DETECT_MONSTERS() = 0;
	virtual const char  *LLOYDS_BEACON() = 0;
	virtual const char  *HOW_MANY_SQUARES() = 0;
	virtual const char  *TOWN_PORTAL() = 0;
	virtual const char  *TOWN_PORTAL_SWORDS() = 0;

	const int TOWN_MAP_NUMBERS[3][5] = {
		{ 28, 29, 30, 31, 32 }, { 29, 31, 33, 35, 37 }, { 53, 92, 63, 0, 0 }
	};

	virtual const char  *MONSTER_DETAILS() = 0;
	virtual const char **MONSTER_SPECIAL_ATTACKS() = 0;
	virtual const char  *IDENTIFY_MONSTERS() = 0;

	const char *const EVENT_SAMPLES[6] = {
		"ahh.voc", "whereto.voc", "gulp.voc", "null.voc", "scream.voc", "laff1.voc"
	};

	virtual const char  *MOONS_NOT_ALIGNED() = 0;
	virtual const char  *AWARDS_FOR() = 0;
	virtual const char  *AWARDS_TEXT() = 0;
	virtual const char  *NO_AWARDS() = 0;
	virtual const char  *WARZONE_BATTLE_MASTER() = 0;
	virtual const char  *WARZONE_MAXED() = 0;
	virtual const char  *WARZONE_LEVEL() = 0;
	virtual const char  *WARZONE_HOW_MANY() = 0;
	virtual const char  *PICKS_THE_LOCK() = 0;
	virtual const char  *UNABLE_TO_PICK_LOCK() = 0;
	virtual const char  *CONTROL_PANEL_TEXT() = 0;
	virtual const char  *CONTROL_PANEL_BUTTONS() = 0;
	virtual const char  *ON() = 0;
	virtual const char  *OFF() = 0;
	virtual const char  *CONFIRM_QUIT() = 0;
	virtual const char  *MR_WIZARD() = 0;
	virtual const char  *NO_LOADING_IN_COMBAT() = 0;
	virtual const char  *NO_SAVING_IN_COMBAT() = 0;
	virtual const char  *QUICK_FIGHT_TEXT() = 0;
	virtual const char **QUICK_FIGHT_OPTIONS() = 0;
	virtual const char **WORLD_END_TEXT() = 0;
	virtual const char  *WORLD_CONGRATULATIONS() = 0;
	virtual const char  *WORLD_CONGRATULATIONS2() = 0;
	virtual const char  *CLOUDS_CONGRATULATIONS1() = 0;
	virtual const char  *CLOUDS_CONGRATULATIONS2() = 0;
	virtual const char **GOOBER() = 0;

	const char *const MUSIC_FILES1[5] = {
		"outdoors.m", "town.m", "cavern.m", "dungeon.m", "castle.m"
	};

	const char *const MUSIC_FILES2[6][7] = {
		{ "outday1.m", "outday2.m", "outday4.m", "outnght1.m",
		"outnght2.m", "outnght4.m", "daydesrt.m" },
		{ "townday1.m", "twnwlk.m", "newbrigh.m", "twnnitea.m",
		"twnniteb.m", "twnwlk.m", "townday1.m" },
		{ "cavern1.m", "cavern2.m", "cavern3a.m", "cavern1.m",
		"cavern2.m", "cavern3a.m", "cavern1.m" },
		{ "dngon1.m", "dngon2.m", "dngon3.m", "dngon1.m",
		"dngon2.m", "dngon3.m", "dngon1.m" },
		{ "cstl1rev.m", "cstl2rev.m", "cstl3rev.m", "cstl1rev.m",
		"cstl2rev.m", "cstl3rev.m", "cstl1rev.m" },
		{ "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m" }
	};

	virtual const char *DIFFICULTY_TEXT() = 0;
	virtual const char *SAVE_OFF_LIMITS() = 0;
	virtual const char *CLOUDS_INTRO1() = 0;
	virtual const char *DARKSIDE_ENDING1() = 0;
	virtual const char *DARKSIDE_ENDING2() = 0;
	virtual const char *PHAROAH_ENDING_TEXT1() = 0;
	virtual const char *PHAROAH_ENDING_TEXT2() = 0;

	void writeConstants(Common::String num, CCArchive &cc);
};

extern void writeConstants(CCArchive &cc);

#endif
