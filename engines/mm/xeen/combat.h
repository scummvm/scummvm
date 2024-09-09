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

#ifndef XEEN_COMBAT_H
#define XEEN_COMBAT_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "mm/xeen/files.h"
#include "mm/xeen/sprites.h"

namespace MM {
namespace Xeen {

#define MAX_NUM_MONSTERS 107
#define PARTY_AND_MONSTERS 12
#define POW_COUNT 12
#define ATTACK_MONSTERS_COUNT 26

enum DamageType {
	DT_PHYSICAL = 0, DT_MAGICAL = 1, DT_FIRE = 2, DT_ELECTRICAL = 3,
	DT_COLD = 4, DT_POISON = 5, DT_ENERGY = 6, DT_SLEEP = 7,
	DT_FINGEROFDEATH = 8, DT_HOLYWORD = 9, DT_MASS_DISTORTION = 10,
	DT_UNDEAD = 11, DT_BEASTMASTER = 12, DT_DRAGONSLEEP = 13,
	DT_GOLEMSTOPPER = 14, DT_HYPNOTIZE = 15, DT_INSECT_SPRAY = 16,
	DT_POISON_VOLLEY = 17, DT_MAGIC_ARROW = 18
};

enum SpecialAttack {
	SA_NONE = 0, SA_MAGIC = 1, SA_FIRE = 2, SA_ELEC = 3, SA_COLD = 4,
	SA_POISON = 5, SA_ENERGY = 6, SA_DISEASE = 7, SA_INSANE = 8,
	SA_SLEEP = 9, SA_CURSEITEM = 10, SA_INLOVE = 11, SA_DRAINSP = 12,
	SA_CURSE = 13, SA_PARALYZE = 14, SA_UNCONSCIOUS = 15,
	SA_CONFUSE = 16, SA_BREAKWEAPON = 17, SA_WEAKEN = 18,
	SA_ERADICATE = 19, SA_AGING = 20, SA_DEATH = 21, SA_STONE = 22
};

enum ShootType {
	ST_0 = 0, ST_1 = 1
};

enum CombatMode {
	COMBATMODE_STARTUP = 0, COMBATMODE_INTERACTIVE = 1, COMBATMODE_2 = 2
};

enum PowType {
	POW_INVALID = -1, POW_FIREBALL = 0, POW_INCINERATE = 1,
	POW_FIERY_FLAIL = 2, POW_LIGHTNING = 3, POW_MEGAVOLTS = 4,
	POW_SPARKS = 5, POW_STOPPER = 6, POW_MAGIC_ORB = 7,
	POW_COLD_RAY = 8, POW_FROST_WAVE = 9, POW_SPRAY = 10,
	POW_ARROW = 11, POW_MAGIC_ARROW = 12, POW_ENERGY_BLAST = 13,
	POW_SPARKLES = 14, POW_DEADLY_SWARM = 15
};

enum RangeType {
	RT_SINGLE = 0, RT_GROUP = 1, RT_ALL = 2, RT_HIT = 3
};

class XeenEngine;
class Character;
class XeenItem;
class MonsterStruct;

struct PowSlot {
	bool _active;
	int _duration;
	int _scale;
	int _elemFrame;
	int _elemScale;

	PowSlot() : _active(false), _duration(0), _scale(0),
		_elemFrame(0), _elemScale(0) {}
};

class PowSlots {
private:
	PowSlot _data[POW_COUNT];
public:
	/**
	 * Gets a slot entry
	 */
	PowSlot &operator[](uint idx) {
		assert(idx < POW_COUNT);
		return _data[idx];
	}

	/**
	 * Resets the elemental frame used in all the slots
	 */
	void resetElementals() {
		for (int idx = 0; idx < POW_COUNT; ++idx)
			_data[idx]._elemFrame = 0;
	}
};

class Combat {
private:
	XeenEngine *_vm;

	/**
	 * Handles the logic for attacking with a given amount of damage
	 */
	void attack2(int damage, RangeType rangeType);

	/**
	 * Returns true if the character successfully hits the monster
	 */
	bool hitMonster(Character &c, RangeType rangeType);

	/**
	 * Gets the damage a given character's equipped weapon will do
	 */
	void getWeaponDamage(Character &c, RangeType rangeType);

	/**
	 * Returns how much damage will be done to a monster
	 */
	int getMonsterDamage(Character &c);

	/**
	 * Gets the scale of damage, used for sprite display in the scene
	 */
	int getDamageScale(int v);

	/**
	 * Gets the current monster's damage resistance to the currently set damage type
	 */
	int getMonsterResistence(RangeType rangeType);

	/**
	 * Distribute experience between active party members
	 */
	void giveExperience(int experience);
public:
	Common::Array<Character *> _combatParty;
	bool _charsBlocked[PARTY_AND_MONSTERS];
	int _charsGone[PARTY_AND_MONSTERS];
	SpriteResource _powSprites;
	int _attackMonsters[ATTACK_MONSTERS_COUNT];
	int _monster2Attack;
	PowSlots _pow;
	int _missedShot[8];
	Common::Array<int> _speedTable;
	int _shootingRow[8];
	int _globalCombat;
	int _whosTurn;
	bool _itemFlag;
	int _monsterMap[32][32];
	bool _monsterMoved[MAX_NUM_MONSTERS];
	bool _rangeAttacking[MAX_NUM_MONSTERS];
	int _gmonHit[36];
	bool _monstersAttacking;
	CombatMode _combatMode;
	int _attackDurationCtr;
	bool _partyRan;
	int _whosSpeed;
	DamageType _damageType;
	Character *_oldCharacter;
	int _monsterDamage;
	int _weaponDamage;
	int _weaponDie, _weaponDice;
	int _weaponElemMaterial;
	XeenItem *_attackWeapon;
	int _attackWeaponId;
	File _missVoc;
	int _hitChanceBonus;
	bool _dangerPresent;
	bool _moveMonsters;
	RangeType _rangeType;
	ShootType _shootType;
	int _combatTarget;
public:
	Combat(XeenEngine *vm);

	/**
	 * Clear the list of attacking monsters
	 */
	void clearAttackers();

	/**
	 * Clear the list of blocked characters
	 */
	void clearBlocked();

	/**
	 * Clear the list of ros projectiles are on headed for part members
	 */
	void clearShooting();

	/**
	 * Resets all combat related data
	 */
	void reset();

	/**
	 * Gives damage to character or characters in the party
	 */
	void giveCharDamage(int damage, DamageType attackType, int charIndex);

	/**
	 * Do damage to a specific character
	 */
	void doCharDamage(Character &c, int charNum, int monsterDataIndex);

	/**
	 * Handles moving monsters by a space between game turns
	 */
	void moveMonsters();

	/**
	 * Setup the combat party with a copy of the currently active party
	 */
	void setupCombatParty();

	/**
	 * Sets up a table of speeds to determine the order in which characters and monsters fight
	 */
	void setSpeedTable();

	/**
	 * Returns true if all participants in the combat are disabled
	 */
	bool allHaveGone() const;

	/**
	 * Returns true if all the characters of the party are disabled
	 */
	bool charsCantAct() const;

	/**
	 * Return a description of the monsters being faced
	 */
	Common::String getMonsterDescriptions();

	/**
	 * Main method for characters to attack
	 */
	void attack(Character &c, RangeType rangeType);

	/**
	 * Flag the currently active character as blocking/defending
	 */
	void block();

	/**
	 * Perform whatever the current combat character's quick action is
	 */
	void quickFight();

	/**
	 * Current selected character is trying to run away
	 */
	void run();

	/**
	 * Called to handle monsters doing ranged attacks against the party
	 */
	void monstersAttack();

	void setupMonsterAttack(int monsterDataIndex, const Common::Point &pt);

	/**
	 * Determines whether a given monster can move
	 * @param pt			Monster position
	 * @param wallShift		Shift mask for determining direction being moved
	 * @param xDiff			X Delta for move
	 * @param yDiff			Y Delta for move
	 * @param monsterId		Monster number being tested
	 */
	bool canMonsterMove(const Common::Point &pt, int wallShift, int xDiff, int yDiff, int monsterId);

	/**
	 * Moves a monster by a given delta amount if it's a valid move
	 */
	void moveMonster(int monsterId, const Common::Point &moveDelta);

	/**
	 * Handle a monster's turn at attacking combat party members
	 */
	void doMonsterTurn(int monsterId);

	/**
	 * Handles a monster's turn at attacking a specific member of the combat party
	 */
	void doMonsterTurn(int monsterId, int charNum);

	/**
	 * Called when combat has ended
	 */
	void endAttack();

	void monsterOvercome();

	/**
	 * Checks whether a given position on the map will stop a ranged attack
	 */
	int stopAttack(const Common::Point &diffPt);

	/**
	 * Called to do ranged attacks, both with bows or using a spell
	 */
	void rangedAttack(PowType powNum);

	/**
	 * Fires off a ranged attack at all oncoming monsters
	 */
	void shootRangedWeapon();

	/**
	 * Returns true if there are any monsters in the vicinity
	 */
	bool areMonstersPresent() const;
};

} // End of namespace Xeen
} // End of namespace MM

#endif
