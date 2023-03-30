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
#include "mm/mm1/game/monster_touch.h"

namespace MM {
namespace MM1 {
namespace Game {

class Combat : public MonsterTouch {
protected:
	Common::Array<Line> _message;
	int _monstersCount = 0;
	Monster *_monsterP;
	bool _treasureFlags[MAX_PARTY_SIZE];
	int _val1;
	int _roundNum;
	int _monsterIndex, _currentChar;
	bool _allowFight, _allowShoot, _allowCast, _allowAttack;
	byte _val6, _val7;
	int _partyIndex, _val9, _monsterShootingCtr;
	int _activeMonsterNum;
	int _destCharCtr;
	int _destAC;
	int _numberOfTimes;
	int _attackerLevel;
	int _advanceIndex;
	int _handicapThreshold, _handicapParty;
	int _handicapMonsters, _handicapDelta;
	int _attackersCount;
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
		CHAR_ATTACKS, MONSTER_ATTACK, INFILTRATION,
		WAITS_FOR_OPENING, SPELL_RESULT, NO_EFFECT
	};
	Mode _mode = SELECT_OPTION;

	int _destMonsterNum = 0;
	int _monstersDestroyedCtr = 0;
	bool _turnUndeadUsed = false;
	bool _divineInterventionUsed = false;
	size_t _spellMonsterCount = 0;

	int _monsterAttackStyle = -1;

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
	 * Does final cleanup when combat is done
	 */
	virtual void combatDone();

	/**
	 * Subtracts the damage from the character, making
	 * them unconscious or die if needed
	 */
	Common::String subtractDamageFromChar() override;

	/**
	 * Clear all the combat variables
	 */
	void clear();

	void loadMonsters();

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
	void setupAttackersCount();

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
	 * Main combat loop that selects the next party
	 * member or monster to take their turn
	 */
	void combatLoop(bool checkMonstersFirst = false);

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
	void monsterAction();

	/**
	 * Check if monster is mindless and wandering
	 */
	bool checkMonsterSpells();

	/**
	 * Check other monster actions
	 */
	void checkMonsterActions();

	void defeatedMonsters();
	void setTreasure();
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

	/**
	 * Handles a monster touch action, if any
	 */
	bool monsterTouch(Common::String &line);

	/**
	 * Handles monster atttack logic
	 */
	void monsterAttackRandom();
	void monsterAttackInner();

	/**
	 * Handles monster shooting a character
	 */
	void monsterAttackShooting();

	/**
	 * Selects monster to attack
	 */
	void selectMonsterTarget();


	/**
	 * Attack option
	 */
	void attack();

	/**
	 * Block option
	 */
	void block();

	/**
	 * Cast option
	 */
	void cast();

	/**
	 * Exchange current character with another
	 */
	void exchangeWith(int charNum);

	/**
	 * Use option
	 */
	void use();

	/**
	 * Retreat option
	 */
	void retreat();

	/**
	 * Called to remove any dead monsters
	 */
	void removeDeadMonsters();

private:
	void spellFailed();
	void destroyMonster();
	bool monsterLevelThreshold() const;
	void iterateMonsters1Inner();
	void iterateMonsters2Inner();
	void characterDone();
	void summonLightning2();
	void fireball2();
	void levelAdjust();

public:
	/**
	 * Display a combat spell's result
	 */
	virtual void displaySpellResult(const InfoMessage &msg) = 0;

	void iterateMonsters1();
	void iterateMonsters2();
	void resetDestMonster();

	// Cleric spells that need access to internal fields
	void turnUndead();
	void summonLightning();
	void paralyze();
	bool divineIntervention();
	void holyWord();

	// Wizard spells that need access to internal fields
	void identifyMonster();
	void fireball();
	void lightningBolt();
	void makeRoom();
	void slow();
	void weaken();
	bool web();
	bool acidRain();
	void fingerOfDeath();
	void disintegration();
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
