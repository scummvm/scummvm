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

#include "mm/mm1/data/spells.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

Spells::SpellFn Spells::SPELLS[SPELLS_COUNT] = {
	// Cleric spells
	placeholder,
	placeholder,
	placeholder,
	cleric14_firstAid,
	cleric15_light,
	cleric16_powerCure,
	cleric17_protectionFromFear,
	placeholder,
	cleric21_cureWounds,
	placeholder,
	placeholder,
	cleric24_protectionFromCold,
	cleric25_protectionFromIce,
	cleric26_protectionFromPoison,
	placeholder,
	placeholder,
	cleric31_createFood,
	cleric32_cureBlindness,
	cleric33_cureParalysis,
	cleric34_lastingLight,
	placeholder,
	placeholder,
	cleric37_removeQuest,
	cleric38_walkOnWater,
	cleric41_cureDisease,
	cleric42_neutralizePoison,
	cleric43_protectionFromAcid,
	cleric44_protectionFromElectricity,
	cleric45_restoreAlignment,
	placeholder,
	placeholder,
	cleric48_surface,
	placeholder,
	cleric52_dispelMagic,
	placeholder,
	cleric54_removeCondition,
	cleric55_restoreEnergy,
	placeholder,
	cleric62_raiseDead,
	cleric63_rejuvinate,
	cleric64_stoneToFlesh,
	cleric65_townPortal,
	placeholder,
	placeholder,
	cleric73_protectionFromElements,
	cleric74_resurrection,
	placeholder,

	placeholder,
	wizard12_detectMagic,
	placeholder,
	placeholder,
	wizard15_leatherSkin,
	wizard16_light,
	wizard17_location,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard24_jump,
	wizard25_levitate,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard32_fly,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	placeholder,
	wizard45_guardDog,
	wizard46_psychicProtection,
	placeholder,
	placeholder,
	placeholder,
	wizard52_dispelMagic,
	placeholder,
	wizard54_shelter,
	wizard55_teleport,
	placeholder,
	placeholder,
	wizard63_etherialize,
	wizard64_protectionFromMagic,
	wizard65_rechargeItem,
	wizard71_astralSpell,
	wizard72_duplication,
	placeholder,
	placeholder,
	placeholder,
};

bool Spells::cast(int spell, Character *chr) {
	assert(spell < SPELLS_COUNT);
	return SPELLS[spell](chr);
}

bool Spells::cleric14_firstAid(Character *chr) {
	return true;
}

bool Spells::cleric15_light(Character *chr) {
	g_globals->_spells._s.light++;
	g_events->send("Game", GameMessage("UPDATE"));
	return true;
}

bool Spells::cleric16_powerCure(Character *chr) { return false; }
bool Spells::cleric17_protectionFromFear(Character *chr) { return false; }
bool Spells::cleric21_cureWounds(Character *chr) { return false; }
bool Spells::cleric24_protectionFromCold(Character *chr) { return false; }
bool Spells::cleric25_protectionFromIce(Character *chr) { return false; }
bool Spells::cleric26_protectionFromPoison(Character *chr) { return false; }
bool Spells::cleric31_createFood(Character *chr) { return false; }
bool Spells::cleric32_cureBlindness(Character *chr) { return false; }
bool Spells::cleric33_cureParalysis(Character *chr) { return false; }
bool Spells::cleric34_lastingLight(Character *chr) { return false; }
bool Spells::cleric37_removeQuest(Character *chr) { return false; }
bool Spells::cleric38_walkOnWater(Character *chr) { return false; }
bool Spells::cleric41_cureDisease(Character *chr) { return false; }
bool Spells::cleric42_neutralizePoison(Character *chr) { return false; }
bool Spells::cleric43_protectionFromAcid(Character *chr) { return false; }
bool Spells::cleric44_protectionFromElectricity(Character *chr) { return false; }
bool Spells::cleric45_restoreAlignment(Character *chr) { return false; }
bool Spells::cleric48_surface(Character *chr) { return false; }
bool Spells::cleric52_dispelMagic(Character *chr) { return false; }
bool Spells::cleric54_removeCondition(Character *chr) { return false; }
bool Spells::cleric55_restoreEnergy(Character *chr) { return false; }
bool Spells::cleric62_raiseDead(Character *chr) { return false; }
bool Spells::cleric63_rejuvinate(Character *chr) { return false; }
bool Spells::cleric64_stoneToFlesh(Character *chr) { return false; }
bool Spells::cleric65_townPortal(Character *chr) { return false; }
bool Spells::cleric73_protectionFromElements(Character *chr) { return false; }
bool Spells::cleric74_resurrection(Character *chr) { return false; }

bool Spells::wizard12_detectMagic(Character *chr) { return false; }
bool Spells::wizard15_leatherSkin(Character *chr) { return false; }
bool Spells::wizard17_location(Character *chr) { return false; }
bool Spells::wizard24_jump(Character *chr) { return false; }
bool Spells::wizard25_levitate(Character *chr) { return false; }
bool Spells::wizard32_fly(Character *chr) { return false; }
bool Spells::wizard45_guardDog(Character *chr) { return false; }
bool Spells::wizard46_psychicProtection(Character *chr) { return false; }
bool Spells::wizard54_shelter(Character *chr) { return false; }
bool Spells::wizard55_teleport(Character *chr) { return false; }
bool Spells::wizard63_etherialize(Character *chr) { return false; }
bool Spells::wizard64_protectionFromMagic(Character *chr) { return false; }
bool Spells::wizard65_rechargeItem(Character *chr) { return false; }
bool Spells::wizard71_astralSpell(Character *chr) { return false; }
bool Spells::wizard72_duplication(Character *chr) { return false; }

} // namespace MM1
} // namespace MM
