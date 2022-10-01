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

#include "mm/mm1/game/spells_monsters.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

const SpellMonstersSpell SpellsMonsters::SPELLS[MONSTER_SPELLS_COUNT] = {
	&SpellsMonsters::spell01_curse,
	&SpellsMonsters::spell02_energyBlast,
	&SpellsMonsters::spell03_fire,
	&SpellsMonsters::spell04_blindness,
	&SpellsMonsters::spell05_sprayPoison,
	&SpellsMonsters::spell06_sprayAcid,
	&SpellsMonsters::spell07_sleep,
	&SpellsMonsters::spell08_paralyze,
	&SpellsMonsters::spell09_dispell,
	&SpellsMonsters::spell10_lightningBolt,
	&SpellsMonsters::spell11_strangeGas,
	&SpellsMonsters::spell12_explode,
	&SpellsMonsters::spell13_fireball,
	&SpellsMonsters::spell14_fire2,
	&SpellsMonsters::spell15_gazes,
	&SpellsMonsters::spell16_acidArrow,
	&SpellsMonsters::spell17_elements,
	&SpellsMonsters::spell18_coldBeam,
	&SpellsMonsters::spell19_dancingSword,
	&SpellsMonsters::spell20_magicDrain,
	&SpellsMonsters::spell21_fingerOfDeath,
	&SpellsMonsters::spell22_sunRay,
	&SpellsMonsters::spell23_disintegration,
	&SpellsMonsters::spell24_commandsEnergy,
	&SpellsMonsters::spell25_poison,
	&SpellsMonsters::spell26_lightning,
	&SpellsMonsters::spell27_frost,
	&SpellsMonsters::spell28_spikes,
	&SpellsMonsters::spell29_acid,
	&SpellsMonsters::spell30_fire,
	&SpellsMonsters::spell31_energy,
	&SpellsMonsters::spell32_swarm
};

void SpellsMonsters::castMonsterSpell(int spellNum) {
	_mmVal1 = _mmVal2 = _mmVal3 = _mmVal4 = 0;
	_monsterSpellMessage = "";
	(this->*SPELLS[spellNum - 1])();
}

void SpellsMonsters::spell01_curse() {
	if (casts())
		g_globals->_spells._s.cursed = MIN(255,
			(int)g_globals->_spells._s.cursed + 1);

	_monsterSpellMessage += STRING["monster_spells.a_curse"];
}

void SpellsMonsters::spell02_energyBlast() {}

void SpellsMonsters::spell03_fire() {}

void SpellsMonsters::spell04_blindness() {}

void SpellsMonsters::spell05_sprayPoison() {}

void SpellsMonsters::spell06_sprayAcid() {}

void SpellsMonsters::spell07_sleep() {}

void SpellsMonsters::spell08_paralyze() {}

void SpellsMonsters::spell09_dispell() {}

void SpellsMonsters::spell10_lightningBolt() {}

void SpellsMonsters::spell11_strangeGas() {}

void SpellsMonsters::spell12_explode() {}

void SpellsMonsters::spell13_fireball() {}

void SpellsMonsters::spell14_fire2() {}

void SpellsMonsters::spell15_gazes() {}

void SpellsMonsters::spell16_acidArrow() {}

void SpellsMonsters::spell17_elements() {}

void SpellsMonsters::spell18_coldBeam() {}

void SpellsMonsters::spell19_dancingSword() {}

void SpellsMonsters::spell20_magicDrain() {}

void SpellsMonsters::spell21_fingerOfDeath() {}

void SpellsMonsters::spell22_sunRay() {}

void SpellsMonsters::spell23_disintegration() {}

void SpellsMonsters::spell24_commandsEnergy() {}

void SpellsMonsters::spell25_poison() {}

void SpellsMonsters::spell26_lightning() {}

void SpellsMonsters::spell27_frost() {}

void SpellsMonsters::spell28_spikes() {}

void SpellsMonsters::spell29_acid() {}

void SpellsMonsters::spell30_fire() {}

void SpellsMonsters::spell31_energy() {}

void SpellsMonsters::spell32_swarm() {}

bool SpellsMonsters::casts() {
	if (canMonsterCast()) {
		_monsterSpellMessage += STRING["monster_spells.casts"];
		return true;
	} else {
		_monsterSpellMessage += STRING["monster_spells.fails_to_cast"];
		return false;
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
