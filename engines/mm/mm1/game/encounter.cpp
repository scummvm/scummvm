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

#include "mm/mm1/game/encounter.h"
#include "mm/mm1/data/monsters.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Game {

void Encounter::execute() {
	if (!g_globals->_encountersOn)
		return;

	Maps::Map &map = *g_maps->_currentMap;
	int comp, maxRand, maxVal;
	const Monster *monsterP;
	_bribeFleeCtr = _bribeAlignmentCtr = 0;
	_alignmentsChanged = 0;

	byte combat1[MAX_COMBAT_MONSTERS];
	Common::fill(&combat1[0], &combat1[MAX_COMBAT_MONSTERS], 0);

	if (!_flag) {
		_monsterList.clear();
		_levelIndex = 0;
	}

	_totalLevels = _highestLevel = 0;
	_levelOffset = _monsterImgNum = _val9 = _fleeThreshold = 0;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		const Character &c = g_globals->_party[i];
		_highestLevel = MAX(_highestLevel, (int)c._level._current);

		if (!(c._condition & (BAD_CONDITION | PARALYZED | UNCONSCIOUS)))
			_totalLevels = MIN(_totalLevels + c._level._current, 255);
	}

	_totalLevels /= 2;
	_highestLevel /= 2;

	bool firstLoop = !_flag;
	_flag = false;

	while (firstLoop || _levelIndex < _totalLevels) {
		randomAdjust();

		maxRand = _levelOffset + _highestLevel;
		if (maxRand >= 2) {
			int highestRand = map[Maps::MAP_33];
			maxRand = MIN(maxRand, highestRand);
			comp = getRandomNumber(maxRand);
		} else {
			comp = 1;
		}

		maxVal = map[Maps::MAP_47];
		if (comp < maxVal) {
			comp = MIN(maxVal, 10);
		}

		assert(_monsterCount < MAX_COMBAT_MONSTERS);
		combat1[_monsterCount] = comp;
		_monsterNum16 = comp;
		_levelIndex += comp;

		_monsterNum = getRandomNumber(16);
		_monsIndexes[_monsterCount] = _monsterNum;
		_monsterCount = (_monsterCount + 1) & 0xff;

		if (_monsterCount < MAX_COMBAT_MONSTERS) {
			if (_monsterCount >= map[Maps::MAP_MAX_MONSTERS])
				goto exit_loop;

			monsterP = getMonster();
			maxVal = getRandomNumber(monsterP->_count);

			for (int i = 0; i < maxVal; ++i) {
				assert(_monsterCount > 0);
				combat1[_monsterCount] = combat1[_monsterCount - 1];
				_levelIndex += combat1[_monsterCount];
				_monsIndexes[_monsterCount] = _monsIndexes[_monsterCount - 1];

				if (++_monsterCount >= MAX_COMBAT_MONSTERS)
					goto exit_loop;

				if (_monsterCount >= map[Maps::MAP_MAX_MONSTERS])
					goto exit_loop;
			}
		} else {
			goto exit_loop;
		}
	}

exit_loop:
	_monsterList.clear();

	for (int i = 0; i < _monsterCount; ++i) {
		maxVal = (combat1[i] - 1) * 16 + _monsIndexes[i];
		if (combat1[i] < 1 || combat1[i] > 12 || maxVal >= 196) {
			combat1[i] = 10;
			_monsIndexes[i] = getRandomNumber(MAX_COMBAT_MONSTERS);
		}

		// Add monster details to list
		_monsterNum16 = combat1[i];
		const Monster &srcMons = g_globals->_monsters[_monsIndexes[i]];
		_monsterList.push_back(srcMons);
		Monster &mons = _monsterList.back();
		mons._combat1 = combat1[i];

		if (_monsterNum16 > _val9) {
			_val9 = _monsterNum16;
			_fleeThreshold = mons._field10;
			_monsterImgNum = mons._field1f;
		}
	}

	g_events->addView("Encounter");
}

void Encounter::randomAdjust() {
	int rval = getRandomNumber(100);
	_levelOffset = 0;

	if (rval < 51) {
	} else if (rval < 71)
		_levelOffset += 1;
	else if (rval < 86)
		_levelOffset += 2;
	else if (rval < 96)
		_levelOffset += 3;
	else
		_levelOffset += 4;
}

const Monster *Encounter::getMonster() {
	assert(_monsterNum > 0 && _monsterNum16 > 0);
	return &g_globals->_monsters[_monsterNum + ((_monsterNum16 - 1) * 16)];
}

bool Encounter::checkSurroundParty() const {
	return getRandomNumber(100) > _fleeThreshold;
}

void Encounter::changeCharAlignment(Alignment align) {
	if (g_globals->_currCharacter->_alignment != align) {
		g_globals->_currCharacter->_alignment = align;
		++_alignmentsChanged;
	}
}

void Encounter::clearMonsters() {
	_monsterList.size();
}

void Encounter::addMonster(byte id, byte arr1) {
	const Monster &mons = g_globals->_monsters[id];
	_monsterList.push_back(mons);
	_monsterList.back()._combat1 = arr1;
}

} // namespace Game
} // namespace MM1
} // namespace MM
