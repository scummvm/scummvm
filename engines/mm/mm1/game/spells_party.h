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

#ifndef MM1_GAME_SPELLS_H
#define MM1_GAME_SPELLS_H

#include "mm/mm1/data/character.h"
#include "mm/mm1/game/game_logic.h"
#include "mm/mm1/messages.h"

namespace MM {
namespace MM1 {
namespace Game {

#define SPELLS_COUNT (47 * 2)

enum SpellResult {
	SR_SUCCESS_DONE, SR_SUCCESS_SILENT, SR_FAILED
};

extern byte FLY_MAP_ID1[20];
extern byte FLY_MAP_ID2[20];
extern byte FLY_MAP_X[20];
extern byte FLY_MAP_Y[20];

class SpellsParty : public GameLogic {
	typedef SpellResult(*SpellFn)();
private:
	static Character *_destChar;
private:
	/**
	 * Returns true if in combat
	 */
	static bool isInCombat();

	/**
	 * Restores an amount of Hp
	 */
	static void restoreHp(uint16 hp);

	/**
	 * Increases light duration
	 */
	static void addLight(int amount);

	/**
	 * Display a message
	 */
	static void display(const InfoMessage &msg);

private:
	static SpellResult placeholder() {
		return SR_FAILED;
	}
	static SpellResult cleric11_awaken();
	static SpellResult cleric12_bless();
	static SpellResult cleric13_blind();
	static SpellResult cleric14_firstAid();
	static SpellResult cleric15_light();
	static SpellResult cleric16_powerCure();
	static SpellResult cleric17_protectionFromFear();
	static SpellResult cleric18_turnUndead();
	static SpellResult cleric21_cureWounds();
	static SpellResult cleric22_heroism();
	static SpellResult cleric23_pain();
	static SpellResult cleric24_protectionFromCold();
	static SpellResult cleric25_protectionFromIce();
	static SpellResult cleric26_protectionFromPoison();
	static SpellResult cleric27_silence();
	static SpellResult cleric28_suggestion();
	static SpellResult cleric31_createFood();
	static SpellResult cleric32_cureBlindness();
	static SpellResult cleric33_cureParalysis();
	static SpellResult cleric34_lastingLight();
	static SpellResult cleric35_produceFlame();
	static SpellResult cleric36_produceFrost();
	static SpellResult cleric37_removeQuest();
	static SpellResult cleric38_walkOnWater();
	static SpellResult cleric41_cureDisease();
	static SpellResult cleric42_neutralizePoison();
	static SpellResult cleric43_protectionFromAcid();
	static SpellResult cleric44_protectionFromElectricity();
	static SpellResult cleric45_restoreAlignment();
	static SpellResult cleric46_summonLightning();
	static SpellResult cleric47_superHeroism();
	static SpellResult cleric48_surface();
	static SpellResult cleric51_deadlySwarm();
	static SpellResult cleric52_dispelMagic();
	static SpellResult cleric54_removeCondition();
	static SpellResult cleric55_restoreEnergy();
	static SpellResult cleric62_raiseDead();
	static SpellResult cleric63_rejuvinate();
	static SpellResult cleric64_stoneToFlesh();
	static SpellResult cleric65_townPortal();
	static SpellResult cleric73_protectionFromElements();
	static SpellResult cleric74_resurrection();

	static SpellResult wizard12_detectMagic();
	static SpellResult wizard15_leatherSkin();
	static SpellResult wizard16_light() {
		return cleric15_light();
	}
	static SpellResult wizard17_location();
	static SpellResult wizard24_jump();
	static SpellResult wizard25_levitate();
	static SpellResult wizard32_fly();
	static SpellResult wizard45_guardDog();
	static SpellResult wizard46_psychicProtection();
	static SpellResult wizard52_dispelMagic() {
		return cleric52_dispelMagic();
	}
	static SpellResult wizard54_shelter();
	static SpellResult wizard55_teleport();
	static SpellResult wizard63_etherialize();
	static SpellResult wizard64_protectionFromMagic();
	static SpellResult wizard65_rechargeItem();
	static SpellResult wizard71_astralSpell();
	static SpellResult wizard72_duplication();

	static SpellFn SPELLS[SPELLS_COUNT];
public:
	/**
	 * Casts a spell
	 */
	static SpellResult cast(uint spell, Character *destChar);
	static SpellResult cast(uint spellx);
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
