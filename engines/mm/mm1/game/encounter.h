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
#include "mm/mm1/data/monsters.h"

namespace MM {
namespace MM1 {
namespace Game {

struct Encounter {
private:
	bool _flag = false;
	int _val1 = 0, _val2 = 0, _val3 = 0;
	int _levelIndex = 0, _val5 = 0, _val6 = 0;
	int _val9 = 0, _val10 = 0;
	int _val11 = 0;
	int _totalLevels = 0, _highestLevel = 0;
	int _randVal = 0;
	byte _arr1[15], _arr2[15];

	void randomAdjust();
	const Monster *getMonster();
	byte getMonsterCount();
public:
	Common::Array<Monster> _monsterList;
	int _val8 = 0;
public:
	void execute();
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
