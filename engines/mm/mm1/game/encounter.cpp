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
	_val1 = _val2 = _val3 = 0;

	if (!_flag) {
		_levelIndex = _val5 = 0;
	}

	_totalLevels = _highestLevel = 0;
	_val6 = _val8 = _val9 = _val10 = 0;

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

		maxRand = _val6 + _highestLevel;
		if (maxRand >= 2) {
			int highestRand = map[33];
			maxRand = MIN(maxRand, highestRand);
			comp = g_engine->getRandomNumber(maxRand);
		} else {
			comp = 1;
		}

		maxVal = map[47];
		if (comp < maxVal) {
			comp = MIN(maxVal, 10);
		}

		assert(_val5 < 15);
		_arr1[_val5] = comp;
		_val11 = comp;
		_levelIndex += comp;

		_randVal = g_engine->getRandomNumber(16);
		_arr2[_val5] = _randVal;
		_val5 = (_val5 + 1) & 0xff;

		if (_val5 < 15) {
			if (_val5 >= map[34])
				goto exit_loop;

			assert(_val11 == 1);
			monsterP = getMonster();
			maxVal = g_engine->getRandomNumber(monsterP->_count);

			for (int i = 0; i < maxVal; ++i) {
				assert(_val5 > 0);
				_arr1[_val5] = _arr1[_val5 - 1];
				_levelIndex += _arr1[_val5];
				_arr2[_val5] = _arr2[_val5 - 1];

				if (++_val5 >= 15)
					goto exit_loop;

				if (_val5 >= map[34])
					goto exit_loop;
			}
		} else {
			goto exit_loop;
		}
	}

exit_loop:
	_monsterList.clear();

	for (int i = 0; i < _val5; ++i) {
		maxVal = (_arr1[i] - 1) * 16 + _arr2[i];
		if (_arr1[i] < 1 || _arr1[i] > 12 || maxVal >= 196) {
			// TODO: This section doesn't make sense, since it will
			// result in an invalid monster offset calculated
			warning("TODO: Nonsensical monster offset set");
			_arr1[i] = 10;
			_arr2[i] = g_engine->getRandomNumber(15);
		}

		// Add monster details to list
		_val11 = _arr1[i];
		assert(_val11 == 1);
		const Monster &mons = g_globals->_monsters[_arr2[i]];
		_monsterList.push_back(mons);

		if (_val11 > _val9) {
			_val9 = _val11;
			_val10 = mons._attr[0];
			_val8 = mons._attr[15];
		}
	}

	g_events->addView("Encounter");
}

void Encounter::randomAdjust() {
	int rval = g_engine->getRandomNumber(100);
	_val6 = 0;

	if (rval < 51) {
	} else if (rval < 71)
		_val6 += 1;
	else if (rval < 86)
		_val6 += 2;
	else if (rval < 96)
		_val6 += 3;
	else
		_val6 += 4;
}

const Monster *Encounter::getMonster() {
	assert(_randVal > 0);
	return &g_globals->_monsters[_randVal];
}

void Encounter::getMonsterVal() {
	assert(_val11 == 1 && _randVal > 0);
	_monsterValP = &g_globals->_monsters[_randVal]._attr[(_val11 - 8) * 2];
}

} // namespace Game
} // namespace MM1
} // namespace MM
