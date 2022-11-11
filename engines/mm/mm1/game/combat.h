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

#ifndef MM1_GAME_COMBAT_H
#define MM1_GAME_COMBAT_H

#include "common/array.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/party.h"
#include "mm/mm1/game/game_logic.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/game/spells_monsters.h"

namespace MM {
namespace MM1 {
namespace Game {

class Combat : public SpellsMonsters {
protected:
	Common::Array<Monster> &_monsterList;
	Common::Array<Line> _message;
	int _monstersCount = 0;
	Common::Array<Character *> _party;
	Monster *_monsterP;
	byte _arr1[MAX_COMBAT_MONSTERS];
	byte _arr2[MAX_COMBAT_MONSTERS];
	int _arr3[MAX_PARTY_SIZE / 2];
	byte _arr4[MAX_COMBAT_MONSTERS];
	bool _canAttack[MAX_PARTY_SIZE];
	bool _treasureFlags[MAX_PARTY_SIZE];
	int _val1;
	int _roundNum;
	int _monsterIndex, _currentChar;
	bool _allowFight, _allowShoot, _allowCast, _allowAttack;
	byte _val6, _val7;
	int _val8, _val9, _val10;
	int _val11, _numberOfTimes;
	int _attackerLevel;
	int _advanceIndex;
	int _handicap1, _handicap2;
	int _handicap3, _handicap4;
	int _attackerVal;
	int _totalExperience;
	Common::String _monsterName;
	bool _monstersResistSpells;
	bool _monstersRegenerate;
	AttributePair _attackAttr1, _attackAttr2;
	int _timesHit;
	bool _isShooting;
	Common::String _attackMessage;
	enum Handicap {
		HANDICAP_EVEN = 0, HANDICAP_PARTY = 1,
		HANDICAP_MONSTER = 2
	};
	Handicap _handicap = HANDICAP_EVEN;
	enum Mode {
		SELECT_OPTION, FIGHT_WHICH, DEFEATED_MONSTERS,
		NEXT_ROUND, MONSTER_ADVANCES, MONSTERS_AFFECTED,
		MONSTER_FLEES, MONSTER_WANDERS, MONSTER_SPELL,
		CHAR_ATTACKS
	};
	Mode _mode = SELECT_OPTION;

	int _destMonsterNum = 0;
	int _monstersDestroyedCtr = 0;
	bool _turnUndeadUsed = false;
	bool _divineInterventionUsed = false;
	size_t _spellMonsterCount = 0;

	/**
	 * Constructor
	 */
	Combat();

	/**
	 * Destructor
	 */
	virtual ~Combat();

	/**
	 * Sets the combat display mode
	 */
	virtual void setMode(Mode newMode) = 0;

	/**
	 * Clear all the combat variables
	 */
	void clear();

	void loadArrays();

	/**
	 * Sets the _monsterIndex to the index of
	 * _monsterP in the monster list
	 */
	void monsterIndexOf();

	/**
	 * Sets _monsterP to point to a specified monster
	 */
	void monsterSetPtr(int monsterNum);

	/**
	 * Sets up the flags for whether each character
	 * in the party can attack from their position.
	 */
	void setupCanAttacks();

	/**
	 * Chooses the starting character to 
	 */
	void setupAttackerVal();

	/**
	 * Checks whether the third party member
	 * is blocked by a left wall
	 */
	void checkLeftWall();

	/**
	 * Checks whether the fourth party member
	 * is blocked by a right wall
	 */
	void checkRightWall();

	/**
	 * Sets up the handicap for the encounter
	 */
	void setupHandicap();

	/*------- Inherited virtual methods ------*/

	/**
	 * Get the monster index
	 */
	int getMonsterIndex() const override {
		return _monsterIndex;
	}

	/**
	 * Returns true if a monster can cast certain spells
	 */
	bool canMonsterCast() const override;

	/**
	 * Dispels any effects on the party
	 */
	void dispelParty() override;

	void removeMonster() override;

	/*------- combat execution ------*/

	/**
	 * Start of the main combat loop
	 */
	void combatLoop();

	/**
	 * Select treasure for a defeated monster
	 */
	void selectTreasure();
	void selectTreasure2(int index);

	/**
	 * Moves to the next round
	 */
	void nextRound();
	void nextRound2();
	void nextRound3();

	/**
	 * Update the _highestLevel to the remaining
	 * active members of the party
	 */
	void updateHighestLevel();

	/**
	 * Calculate a monster action
	 */
	void checkMonsterFlees();

	/**
	 * Check if monster is mindless and wandering
	 */
	void checkMonsterSpells();

	/**
	 * Check other monster actions
	 */
	void checkMonsterActions();

	void defeatedMonsters();
	void selectParty();
	void loop1();
	void proc1();
	void clearArrays();
	bool moveMonsters();
	void monsterAdvances();
	bool monsterChanges();
	void proc2();
	void checkParty();

	/**
	 * Attack a monster
	 */
	void fightMonster(int monsterNum);
	void shootMonster(int monsterNum);
	void attackMonsterPhysical();
	void attackMonsterShooting();
	void attackMonster(int monsterNum);

	/**
	 * Adds attack damage message for character hitting monster
	 */
	void addAttackDamage();

	/**
	 * Updates a monster's status
	 */
	void updateMonsterStatus();

private:
	void spellFailed();
	void destroyUndead();
	bool monsterLevelThreshold() const;
	void iterateMonsters1Inner();
	void iterateMonsters2Inner();
	void updateArr3();
	void summonLightning2();

public:
	/**
	 * Display a combat spell's result
	 */
	virtual void displaySpellResult(const Common::Array<InfoMessage> &msgs);
	void displaySpellResult(const InfoMessage &msg);

	void iterateMonsters1();
	void iterateMonsters2();
	void resetDestMonster();

	void turnUndead();
	void summonLightning();
	void paralyze();
	bool divineIntervention();
	void holyWord();
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
