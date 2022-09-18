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
	Maps::Map &map = *g_maps->_currentMap;
	int comp, maxRand, maxVal;
	const Monster *monsterP;
	_bribeFleeCtr = _bribeAlignmentCtr = 0;
	_alignmentsChanged = 0;

	if (!_flag) {
		_levelIndex = _monsterIndex = 0;
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
			int highestRand = map[33];
			maxRand = MIN(maxRand, highestRand);
			comp = g_engine->getRandomNumber(1, maxRand);
		} else {
			comp = 1;
		}

		maxVal = map[47];
		if (comp < maxVal) {
			comp = MIN(maxVal, 10);
		}

		assert(_monsterIndex < 15);
		_arr1[_monsterIndex] = comp;
		_monsterNum16 = comp;
		_levelIndex += comp;

		_monsterNum = g_engine->getRandomNumber(1, 16);
		_arr2[_monsterIndex] = _monsterNum;
		_monsterIndex = (_monsterIndex + 1) & 0xff;

		if (_monsterIndex < 15) {
			if (_monsterIndex >= map[34])
				goto exit_loop;

			monsterP = getMonster();
			maxVal = g_engine->getRandomNumber(1, monsterP->_count);

			for (int i = 0; i < maxVal; ++i) {
				assert(_monsterIndex > 0);
				_arr1[_monsterIndex] = _arr1[_monsterIndex - 1];
				_levelIndex += _arr1[_monsterIndex];
				_arr2[_monsterIndex] = _arr2[_monsterIndex - 1];

				if (++_monsterIndex >= 15)
					goto exit_loop;

				if (_monsterIndex >= map[34])
					goto exit_loop;
			}
		} else {
			goto exit_loop;
		}
	}

exit_loop:
	_monsterList.clear();

	for (int i = 0; i < _monsterIndex; ++i) {
		maxVal = (_arr1[i] - 1) * 16 + _arr2[i];
		if (_arr1[i] < 1 || _arr1[i] > 12 || maxVal >= 196) {
			_arr1[i] = 10;
			_arr2[i] = g_engine->getRandomNumber(1, 15);
		}

		// Add monster details to list
		_monsterNum16 = _arr1[i];
		const Monster &mons = g_globals->_monsters[_arr2[i]];
		_monsterList.push_back(mons);

		if (_monsterNum16 > _val9) {
			_val9 = _monsterNum16;
			_fleeThreshold = mons._unk[0];
			_monsterImgNum = mons._unk[15];
		}
	}

	g_events->addView("Encounter");
}

void Encounter::randomAdjust() {
	int rval = g_engine->getRandomNumber(1, 100);
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
	return g_engine->getRandomNumber(1, 100) > _fleeThreshold;
}

void Encounter::changeCharAlignment(Alignment align) {
	if (g_globals->_currCharacter->_alignment != align) {
		g_globals->_currCharacter->_alignment = align;
		++_alignmentsChanged;
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
