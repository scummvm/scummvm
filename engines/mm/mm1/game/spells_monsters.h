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
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/monsters.h"
#include "mm/mm1/messages.h"
#include "common/str.h"

namespace MM {
namespace MM1 {
namespace Game {

#define MAX_COMBAT_MONSTERS 15
#define MONSTER_SPELLS_COUNT 32

class SpellsMonsters;
typedef void (SpellsMonsters::*SpellMonstersSpell)();

class SpellsMonsters : public GameLogic {
private:
	static const SpellMonstersSpell SPELLS[MONSTER_SPELLS_COUNT];

	void spell01_curse();
	void spell02_energyBlast();
	void spell03_fire();
	void spell04_blindness();
	void spell05_sprayPoison();
	void spell06_sprayAcid();
	void spell07_sleep();
	void spell08_paralyze();
	void spell09_dispel();
	void spell10_lightningBolt();
	void spell11_strangeGas();
	void spell12_explode();
	void spell13_fireball();
	void spell14_fireBreath();
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

	/**
	 * Called to determine whether the spell can be cast
	 */
	bool casts();

	/**
	 * Adds text to the current line
	 */
	void add(const Common::String &msg) {
		_lines.back()._text += msg;
	}
	void add(char c) {
		_lines.back()._text += c;
	}

	/**
	 * Adds current character's name
	 */
	void addCharName();

	/**
	 * Selects a random character and applies the damage to them
	 */
	void damageRandomChar();

	/**
	 * Randomly chooses a character in the party
	 */
	void chooseCharacter();

	/**
	 * Checks whether the character is affected by the spell,
	 * and if so writes out the damage and whether the
	 * character is knocked unconscious or dies
	 */
	void handleDamage();

	/**
	 * Checks if character is affected by spell,
	 * and adds to the message if not
	 */
	bool charAffected();

	/**
	 * Checks random range
	 */
	bool randomThreshold(int threshold) const {
		int v = getRandomNumber(120);
		return v < 3 || v >= threshold;
	}

	bool isEffective();

	/**
	 * Writes out how much damage the character suffers
	 */
	void writeDamage();

	bool testElementalResistance();

	/**
	 * Adds different spell effects to the lines
	 */
	void writeConditionEffect();

	/**
	 * Adds text for condition effects on the party
	 */
	void handlePartyEffects();

protected:
	Common::Array<Monster *> _remainingMonsters;
	LineArray _lines;
	int _damage = 0;

	virtual bool canMonsterCast() const = 0;
	virtual int getMonsterIndex() const = 0;
	virtual void dispelParty() = 0;
	virtual void removeMonster() = 0;

	/**
	 * Adds text for damage effects on the party
	 */
	void handlePartyDamage();

	/**
	 * Sets the condition to apply
	 */
	void setCondition(byte newCondition);

	/**
	 * Returns true if character is affected so spell
	 */
	bool isCharAffected() const;

	/**
	 * Test whether character resists different damage types
	 */
	bool damageType1();
	bool damageType2();
	bool damageType3();
	bool damageType4();
	bool damageType5();
	bool damageType6();
	bool damageType7();

	void proc9();

	/**
	 * Subtracts the damage from the character, making
	 * them unconscious or die if needed
	 */
	void subtractDamage();

public:
	SpellsMonsters();
	virtual ~SpellsMonsters() {}

	/**
	 * Monster casts a spell
	 */
	void castMonsterSpell(const Common::String &monsterName, int spellNum);

	/**
	 * Gets a spell message
	 */
	const LineArray &getMonsterSpellMessage() const {
		return _lines;
	}
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
