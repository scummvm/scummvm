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

namespace MM {
namespace MM1 {
namespace Game {

class Combat : public GameLogic {
protected:
	int _monstersCount = 0;
	Common::Array<Character *> _party;
	Monster *_monsterP;
	byte _arr1[MAX_COMBAT_MONSTERS];
	byte _arr2[MAX_COMBAT_MONSTERS];
	byte _arr3[MAX_COMBAT_MONSTERS];
	bool _canAttack[MAX_PARTY_SIZE];
	int _val1, _val8, _val9, _val10;
	int _monsterIndex, _currentChar;
	char _val2, _val3, _val4, _val5;
	int _attackerVal;

	Combat() { clear(); }

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
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
