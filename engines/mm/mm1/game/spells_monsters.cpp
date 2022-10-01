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
	if (casts()) {
		_monsterSpellMessage += STRING["monster_spells.a_curse"];
		g_globals->_spells._s.cursed = MIN(255,
			(int)g_globals->_spells._s.cursed + 1);
	}
}

void SpellsMonsters::spell02_energyBlast() {
	if (casts()) {		
		_monsterSpellMessage += STRING["monster_spells.energy_blast"];
		++_mmVal1;
		_mmVal4 = getRandomNumber(16) + 4;
		proc1();
	}
}

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

void SpellsMonsters::proc1() {
	chooseCharacter();
	proc3();
}

void SpellsMonsters::chooseCharacter() {
	_monsterSpellMessage += ':';

	// Choose a random character
	g_globals->_currCharacter = &g_globals->_party[
		getRandomNumber(g_globals->_party.size()) - 1
	];

	if (g_globals->_currCharacter->_condition & (BAD_CONDITION | UNCONSCIOUS)) {
		// Can't use character, so sequently scan party
		// to find a character that can be used
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_currCharacter = &g_globals->_party[i];
			if (g_globals->_currCharacter->_condition & (BAD_CONDITION | UNCONSCIOUS))
				break;
		}
	}
}

void SpellsMonsters::proc3() {
	_mmVal5 = 1;
	_mmVal6 = _mmVal4;
	proc4();
	proc5();
	proc6();

	if (g_globals->_spells._s.power_shield)
		_mmVal6 = 1;

	proc7();
	proc8();
}

void SpellsMonsters::proc4() {

}

void SpellsMonsters::proc5() {

}

void SpellsMonsters::proc6() {

}

void SpellsMonsters::proc7() {

}

void SpellsMonsters::proc8() {

}

} // namespace Game
} // namespace MM1
} // namespace MM
