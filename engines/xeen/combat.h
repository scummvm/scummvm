/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_COMBAT_H
#define XEEN_COMBAT_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "xeen/files.h"
#include "xeen/sprites.h"

namespace Xeen {

#define MAX_NUM_MONSTERS 107
#define PARTY_AND_MONSTERS 12
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

enum ElementalCategory {
	ELEM_FIRE = 0, ELEM_ELECTRICITY = 1, ELEM_COLD = 2,
	ELEM_ACID_POISON = 3, ELEM_ENERGY = 4, ELEM_MAGIC = 5
};

enum RangeType {
	RT_SINGLE = 0, RT_GROUP = 1, RT_ALL = 2, RT_3 = 3
};

enum ShootType {
	ST_0 = 0, ST_1 = 1
};

enum CombatMode {
	COMBATMODE_0 = 0, COMBATMODE_1 = 1, COMBATMODE_2 = 2
};

enum PowType {
	POW_INVALID = -1, POW_FIREBALL = 0, POW_INCINERATE = 1,
	POW_FIERY_FLAIL = 2, POW_LIGHTNING = 3, POW_MEGAVOLTS = 4,
	POW_SPARKS = 5, POW_STOPPER = 6, POW_MAGIC_ORB = 7,
	POW_COLD_RAY = 8, POW_FROST_WAVE = 9, POW_SPRAY = 10,
	POW_ARROW = 11, POW_MAGIC_ARROW = 12, POW_ENERGY_BLAST = 13,
	POW_SPARKLES = 14, POW_DEADLY_SWARM = 15
};

class XeenEngine;
class Character;
class XeenItem;

class Combat {
private:
	XeenEngine *_vm;

	void attack2(int damage, RangeType rangeType);

	bool hitMonster(Character &c, RangeType rangeType);

	void getWeaponDamage(Character &c, RangeType rangeType);

	int getMonsterDamage(Character &c);

	int getDamageScale(int v);

	int getMonsterResistence(RangeType rangeType);

	/**
	 * Distribute experience between active party members
	 */
	void giveExperience(int experience);
public:
	Common::Array<Character *> _combatParty;
	bool _charsBlocked[PARTY_AND_MONSTERS];
	Common::Array<int> _charsGone;
	SpriteResource _powSprites;
	int _attackMonsters[ATTACK_MONSTERS_COUNT];
	int _monster2Attack;
	int _charsArray1[PARTY_AND_MONSTERS];
	bool _monPow[PARTY_AND_MONSTERS];
	int _monsterScale[PARTY_AND_MONSTERS];
	ElementalCategory _elemPow[PARTY_AND_MONSTERS];
	int _elemScale[PARTY_AND_MONSTERS];
	int _missedShot[8];
	Common::Array<int> _speedTable;
	int _shooting[8];
	int _globalCombat;
	int _whosTurn;
	bool _itemFlag;
	int _monsterMap[32][32];
	bool _monsterMoved[MAX_NUM_MONSTERS];
	bool _rangeAttacking[MAX_NUM_MONSTERS];
	int _gmonHit[36];
	bool _monstersAttacking;
	CombatMode _combatMode;
	int _monsterIndex;
	bool _partyRan;
	int _whosSpeed;
	DamageType _damageType;
	Character *_oldCharacter;
	int _monsterDamage;
	int _weaponDamage;
	int _weaponDie, _weaponDice;
	XeenItem *_attackWeapon;
	int _attackWeaponId;
	File _missVoc, _pow1Voc;
	int _hitChanceBonus;
	bool _dangerPresent;
	bool _moveMonsters;
	RangeType _rangeType;
	ShootType _shootType;
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

	void giveCharDamage(int damage, DamageType attackType, int charIndex);

	/**
	 * Do damage to a specific character
	 */
	void doCharDamage(Character &c, int charNum, int monsterDataIndex);

	void moveMonsters();

	/**
	 * Setup the combat party with a copy of the currently active party
	 */
	void setupCombatParty();

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
	 */
	bool monsterCanMove(const Common::Point &pt, int wallShift,
		int v1, int v2, int monsterId);

	/**
	 * Moves a monster by a given delta amount if it's a valid move
	 */
	void moveMonster(int monsterId, const Common::Point &moveDelta);

	void doMonsterTurn(int monsterId);

	void endAttack();

	void monsterOvercome();

	int stopAttack(const Common::Point &diffPt);

	/**
	 * Called to do ranged attacks, both with bows or using a spell
	 */
	void rangedAttack(PowType powNum);

	/**
	 * Fires off a ranged attack at all oncoming monsters
	 */
	void shootRangedWeapon();
};

} // End of namespace Xeen

#endif	/* XEEN_COMBAT_H */
