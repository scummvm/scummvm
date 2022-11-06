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

class SpellsParty {
	typedef SpellResult(*SpellFn)(Character *chr);
private:
	/**
	 * Restores an amount of Hp
	 */
	static void restoreHp(Character *chr, uint16 hp);

	/**
	 * Increases light duration
	 */
	static void addLight(int amount);

	static void iterateMonsters();

private:
	static SpellResult placeholder(Character *chr) {
		return SR_FAILED;
	}
	static SpellResult cleric11_awaken(Character *chr);
	static SpellResult cleric12_bless(Character *chr);
	static SpellResult cleric13_blind(Character *chr);
	static SpellResult cleric14_firstAid(Character *chr);
	static SpellResult cleric15_light(Character *chr);
	static SpellResult cleric16_powerCure(Character *chr);
	static SpellResult cleric17_protectionFromFear(Character *chr);
	static SpellResult cleric21_cureWounds(Character *chr);
	static SpellResult cleric24_protectionFromCold(Character *chr);
	static SpellResult cleric25_protectionFromIce(Character *chr);
	static SpellResult cleric26_protectionFromPoison(Character *chr);
	static SpellResult cleric31_createFood(Character *chr);
	static SpellResult cleric32_cureBlindness(Character *chr);
	static SpellResult cleric33_cureParalysis(Character *chr);
	static SpellResult cleric34_lastingLight(Character *chr);
	static SpellResult cleric37_removeQuest(Character *chr);
	static SpellResult cleric38_walkOnWater(Character *chr);
	static SpellResult cleric41_cureDisease(Character *chr);
	static SpellResult cleric42_neutralizePoison(Character *chr);
	static SpellResult cleric43_protectionFromAcid(Character *chr);
	static SpellResult cleric44_protectionFromElectricity(Character *chr);
	static SpellResult cleric45_restoreAlignment(Character *chr);
	static SpellResult cleric48_surface(Character *chr);
	static SpellResult cleric52_dispelMagic(Character *chr);
	static SpellResult cleric54_removeCondition(Character *chr);
	static SpellResult cleric55_restoreEnergy(Character *chr);
	static SpellResult cleric62_raiseDead(Character *chr);
	static SpellResult cleric63_rejuvinate(Character *chr);
	static SpellResult cleric64_stoneToFlesh(Character *chr);
	static SpellResult cleric65_townPortal(Character *chr);
	static SpellResult cleric73_protectionFromElements(Character *chr);
	static SpellResult cleric74_resurrection(Character *chr);

	static SpellResult wizard12_detectMagic(Character *chr);
	static SpellResult wizard15_leatherSkin(Character *chr);
	static SpellResult wizard16_light(Character *chr) {
		return cleric15_light(chr);
	}
	static SpellResult wizard17_location(Character *chr);
	static SpellResult wizard24_jump(Character *chr);
	static SpellResult wizard25_levitate(Character *chr);
	static SpellResult wizard32_fly(Character *chr);
	static SpellResult wizard45_guardDog(Character *chr);
	static SpellResult wizard46_psychicProtection(Character *chr);
	static SpellResult wizard52_dispelMagic(Character *chr) {
		return cleric52_dispelMagic(chr);
	}
	static SpellResult wizard54_shelter(Character *chr);
	static SpellResult wizard55_teleport(Character *chr);
	static SpellResult wizard63_etherialize(Character *chr);
	static SpellResult wizard64_protectionFromMagic(Character *chr);
	static SpellResult wizard65_rechargeItem(Character *chr);
	static SpellResult wizard71_astralSpell(Character *chr);
	static SpellResult wizard72_duplication(Character *chr);

	static SpellFn SPELLS[SPELLS_COUNT];
public:
	/**
	 * Casts a spell
	 * @returns If false, display a 'Done' message.
	 * If true, spell has done it's own UI display
	 */
	static SpellResult cast(int spell, Character *chr);
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
