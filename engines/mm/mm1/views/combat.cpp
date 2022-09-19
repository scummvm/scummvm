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

#include "mm/mm1/views/combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Combat::Combat() : TextView("Combat") {
}

bool Combat::msgFocus(const FocusMessage &msg) {
	Game::Encounter &enc = g_globals->_encounters;
	_mode = BATTLE;

	// Clear combat data
	clear();

	_monstersCount = enc._monsterList.size();
	_party.clear();
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		_party.push_back(&g_globals->_party[i]);

	loadArrays();

	return true;
}

void Combat::clear() {
	Common::fill(&_arr1[0], &_arr1[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_arr2[0], &_arr2[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_arr3[0], &_arr3[MAX_COMBAT_MONSTERS], 0);
	Common::fill(&_arr4[0], &_arr4[MAX_PARTY_SIZE], 0);
	_val1 = _val8 = _val9 = _val10 = 0;
	_monsterP = nullptr;
	_monsterIndex = 0;
	// TODO: clear everything
}

void Combat::loadArrays() {
	Game::Encounter &enc = g_globals->_encounters;

	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		Monster &mon = enc._monsterList[i];
		int val = getRandomNumber(1, 8);

		mon._field11 += val;
		_arr1[i] = mon._field11;
		_arr2[i] = mon._field12;

		monsterIndexOf();
	}
}

void Combat::monsterIndexOf() {
	Game::Encounter &enc = g_globals->_encounters;

	_monsterIndex = MAX_COMBAT_MONSTERS;
	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		if (_monsterP == &enc._monsterList[i]) {
			_monsterIndex = i;
			break;
		}
	}
}

void Combat::draw() {
	//Game::Encounter &enc = g_globals->_encounters;

	clearSurface();
	writeString(10, 10, "TODO: COMBAT VIEW");
}

void Combat::timeout() {
	redraw();
}

bool Combat::msgKeypress(const KeypressMessage &msg) {
	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
