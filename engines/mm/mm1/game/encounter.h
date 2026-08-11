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

#ifndef MM1_GAME_ENCOUNTER_H
#define MM1_GAME_ENCOUNTER_H

#include "common/array.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/monsters.h"
#include "mm/mm1/game/game_logic.h"

namespace MM {
namespace MM1 {
namespace Game {

#define MAX_COMBAT_MONSTERS 15

enum EncounterType {
	FORCE_SURPRISED = -1, NORMAL_SURPRISED = 0, NORMAL_ENCOUNTER = 1
};

class Encounter : public GameLogic {
private:
	int _levelOffset = 0;
	int _maxLevelForImg = 0;
	int _monsterLevel = 0;
	int _totalLevels = 0;
	int _monsterNum = 0;
	struct MonsterSummary {
		byte _num;
		byte _level;
		MonsterSummary() : _num(0), _level(0) {}
		MonsterSummary(byte num, byte level) : _num(num), _level(level) {}
	};
	Common::Array<MonsterSummary> _monsterSummaries;

	void randomAdjust();
	const Monster *getMonster();
public:
	Common::Array<Monster> _monsterList;
	int _bribeAlignmentCtr = 0, _bribeFleeCtr = 0;
	int _alignmentsChanged = 0;
	int _monsterImgNum = 0;
	int _highestLevel = 0;
	EncounterType _encounterType = NORMAL_SURPRISED;
	byte _fleeThreshold = 0;
	bool _manual = false;
	int _levelIndex = 0;

public:
	/**
	 * Start an encounter
	 */
	void execute();

	/**
	 * Chooses whether an encounter can be fled
	 */
	bool checkSurroundParty() const;

	void changeCharAlignment(Alignment align);

	/**
	 * Clears the monster list
	 */
	void clearMonsters();

	/**
	 * Adds a monster to the monster list
	 */
	void addMonster(byte id, byte level);
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
