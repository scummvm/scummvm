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

#ifndef MM1_GAME_SPELLS_MONSTERS
#define MM1_GAME_SPELLS_MONSTERS

#include "mm/mm1/game/game_logic.h"
#include "common/str.h"

namespace MM {
namespace MM1 {
namespace Game {

#define MONSTER_SPELLS_COUNT 32

class SpellsMonsters;
typedef void (SpellsMonsters::*SpellMonstersSpell)();

class SpellsMonsters : public GameLogic {
private:
	static const SpellMonstersSpell SPELLS[MONSTER_SPELLS_COUNT];
	Common::String _monsterSpellMessage;
	int _mmVal1 = 0, _mmVal2 = 0;
	int _mmVal3 = 0, _mmVal4 = 0;
	int _mmVal5 = 0, _mmVal6 = 0;

	void spell01_curse();
	void spell02_energyBlast();
	void spell03_fire();
	void spell04_blindness();
	void spell05_sprayPoison();
	void spell06_sprayAcid();
	void spell07_sleep();
	void spell08_paralyze();
	void spell09_dispell();
	void spell10_lightningBolt();
	void spell11_strangeGas();
	void spell12_explode();
	void spell13_fireball();
	void spell14_fire2();
	void spell15_gazes();
	void spell16_acidArrow();
	void spell17_elements();
	void spell18_coldBeam();
	void spell19_dancingSword();
	void spell20_magicDrain();
	void spell21_fingerOfDeath();
	void spell22_sunRay();
	void spell23_disintegration();
	void spell24_commandsEnergy();
	void spell25_poison();
	void spell26_lightning();
	void spell27_frost();
	void spell28_spikes();
	void spell29_acid();
	void spell30_fire();
	void spell31_energy();
	void spell32_swarm();

	bool casts();
	void proc1();
	void chooseCharacter();
	void proc3();
	void proc4();
	void proc5();
	void proc6();
	void proc7();
	void proc8();

protected:
	virtual bool canMonsterCast() const = 0;

public:
	/**
	 * Monster casts a spell
	 */
	void castMonsterSpell(int spellNum);

	/**
	 * Gets a spell message
	 */
	Common::String getMonsterSpellMessage() const {
		return _monsterSpellMessage;
	}
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
