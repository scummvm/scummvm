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

	// In manual mode, the scripts have already set up
	// a list of monsters to use
	if (!_manual) {
		_monsterSummaries.clear();
		_levelIndex = 0;
	}

	_totalLevels = _highestLevel = 0;
	_levelOffset = _monsterImgNum = _maxLevelForImg = _fleeThreshold = 0;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		const Character &c = g_globals->_party[i];
		_highestLevel = MAX(_highestLevel, (int)c._level._current);

		if (!(c._condition & (BAD_CONDITION | PARALYZED | UNCONSCIOUS)))
			_totalLevels = MIN(_totalLevels + c._level._current, 255);
	}

	_totalLevels /= 2;
	_highestLevel /= 2;

	bool firstLoop = !_manual;
	_manual = false;

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

		assert(_monsterSummaries.size() < MAX_COMBAT_MONSTERS);
		_monsterNum = getRandomNumber(16);
		_monsterSummaries.push_back(MonsterSummary(_monsterNum, comp));

		_monsterLevel = comp;
		_levelIndex += comp;

		if (_monsterSummaries.size() < MAX_COMBAT_MONSTERS) {
			if (_monsterSummaries.size() >= map[Maps::MAP_MAX_MONSTERS])
				goto exit_loop;

			monsterP = getMonster();
			maxVal = getRandomNumber(monsterP->_count);

			for (int i = 0; i < maxVal; ++i) {
				assert(!_monsterSummaries.empty());
				_monsterSummaries.push_back(_monsterSummaries.back());
				_levelIndex += _monsterSummaries.back()._level;

				if (_monsterSummaries.size() >= MAX_COMBAT_MONSTERS)
					goto exit_loop;

				if (_monsterSummaries.size() >= map[Maps::MAP_MAX_MONSTERS])
					goto exit_loop;
			}
		} else {
			goto exit_loop;
		}
	}

exit_loop:
	_monsterList.clear();

	for (uint i = 0; i < _monsterSummaries.size(); ++i) {
		maxVal = (_monsterSummaries[i]._level - 1) * 16 +
			_monsterSummaries[i]._num;

		if (_monsterSummaries[i]._level < 1 || _monsterSummaries[i]._level > 12
				|| maxVal >= 196) {
			_monsterSummaries[i]._level = 10;
			_monsterSummaries[i]._num = getRandomNumber(15);
		}

		// Add monster details to list
		_monsterLevel = _monsterSummaries[i]._level;
		const Monster &srcMons = (*g_globals->_monsters)[_monsterSummaries[i]._num];
		_monsterList.push_back(srcMons);
		Monster &mons = _monsterList.back();
		mons._level = _monsterSummaries[i]._level;

		if (_monsterLevel > _maxLevelForImg) {
			_maxLevelForImg = _monsterLevel;
			_fleeThreshold = mons._fleeThreshold;
			_monsterImgNum = mons._imgNum;
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
	assert(_monsterNum > 0 && _monsterLevel > 0);
	return &(*g_globals->_monsters)[_monsterNum + ((_monsterLevel - 1) * 16)];
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
	_monsterSummaries.clear();
}

void Encounter::addMonster(byte id, byte level) {
	_monsterSummaries.push_back(MonsterSummary(id, level));
}

} // namespace Game
} // namespace MM1
} // namespace MM
