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

#ifndef MM1_DATA_SPELLS_H
#define MM1_DATA_SPELLS_H

#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {

#define SPELLS_COUNT (47 * 2)
#define ACTIVE_SPELLS_COUNT 18

struct ActiveSpellsStruct {
	byte fear;
	byte cold;
	byte fire;
	byte poison;
	byte acid;
	byte elec;
	byte magic;
	byte light;
	byte leather_skin;
	byte levitate;
	byte walk_on_water;
	byte guard_dog;
	byte psychic_protecti;
	byte bless;
	byte invisbility;
	byte shield;
	byte power_shield;
	byte cursed;
};

union ActiveSpells {
	ActiveSpellsStruct _s;
	byte _arr[ACTIVE_SPELLS_COUNT];
};

class Spells {
	typedef bool (*SpellFn)(Character *chr);
private:
	static bool placeholder(Character *chr) { return false; }
	static bool cleric14_firstAid(Character *chr);
	static bool cleric15_light(Character *chr);
	static bool cleric16_powerCure(Character *chr);
	static bool cleric17_protectionFromFear(Character *chr);
	static bool cleric21_cureWounds(Character *chr);
	static bool cleric24_protectionFromCold(Character *chr);
	static bool cleric25_protectionFromIce(Character *chr);
	static bool cleric26_protectionFromPoison(Character *chr);
	static bool cleric31_createFood(Character *chr);
	static bool cleric32_cureBlindness(Character *chr);
	static bool cleric33_cureParalysis(Character *chr);
	static bool cleric34_lastingLight(Character *chr);
	static bool cleric37_removeQuest(Character *chr);
	static bool cleric38_walkOnWater(Character *chr);
	static bool cleric41_cureDisease(Character *chr);
	static bool cleric42_neutralizePoison(Character *chr);
	static bool cleric43_protectionFromAcid(Character *chr);
	static bool cleric44_protectionFromElectricity(Character *chr);
	static bool cleric45_restoreAlignment(Character *chr);
	static bool cleric48_surface(Character *chr);
	static bool cleric52_dispelMagic(Character *chr);
	static bool cleric54_removeCondition(Character *chr);
	static bool cleric55_restoreEnergy(Character *chr);
	static bool cleric62_raiseDead(Character *chr);
	static bool cleric63_rejuvinate(Character *chr);
	static bool cleric64_stoneToFlesh(Character *chr);
	static bool cleric65_townPortal(Character *chr);
	static bool cleric73_protectionFromElements(Character *chr);
	static bool cleric74_resurrection(Character *chr);

	static bool wizard12_detectMagic(Character *chr);
	static bool wizard15_leatherSkin(Character *chr);
	static bool wizard16_light(Character *chr) {
		return cleric15_light(chr);
	}
	static bool wizard17_location(Character *chr);
	static bool wizard24_jump(Character *chr);
	static bool wizard25_levitate(Character *chr);
	static bool wizard32_fly(Character *chr);
	static bool wizard45_guardDog(Character *chr);
	static bool wizard46_psychicProtection(Character *chr);
	static bool wizard52_dispelMagic(Character *chr) {
		return cleric52_dispelMagic(chr);
	}
	static bool wizard54_shelter(Character *chr);
	static bool wizard55_teleport(Character *chr);
	static bool wizard63_etherialize(Character *chr);
	static bool wizard64_protectionFromMagic(Character *chr);
	static bool wizard65_rechargeItem(Character *chr);
	static bool wizard71_astralSpell(Character *chr);
	static bool wizard72_duplication(Character *chr);

	static SpellFn SPELLS[SPELLS_COUNT];
public:
	/**
	 * Casts a spell
	 * @returns If false, display a 'Done' message.
	 * If true, spell has done it's own UI display
	 */
	static bool cast(int spell, Character *chr);
};

} // namespace MM1
} // namespace MM

#endif
