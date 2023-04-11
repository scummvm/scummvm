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

#include "mm/mm1/game/wheel_spin.h"
#include "mm/mm1/maps/map16.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

#define VAL2 84
#define VAL3 85
#define VAL4 87

void WheelSpin::spin() {
	Maps::Map16 &map = *static_cast<Maps::Map16 *>(g_maps->_currentMap);
	_results.clear();


	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;
		map[VAL2] = i;

		// Count set flags
		map[VAL4] = 0;
		byte v = c._flags[2];
		for (int j = 0; j < 4; ++j, v >>= 1) {
			if (v & 1)
				map[VAL4]++;
		}

		Common::String line;

		if (map[VAL4] == 0) {
			line = STRING["maps.map16.loser"];
		} else {
			c._flags[2] |= CHARFLAG2_80;
			int val;

			switch (getRandomNumber(6)) {
			case 1:
				val = 2000 << map[VAL4];
				WRITE_LE_UINT16(&map[VAL3], val);
				c._exp += val;
				line = Common::String::format("+%d %s", val,
					STRING["maps.map16.exp"].c_str());
				break;

			case 2:
				val = 500 << map[VAL4];
				WRITE_LE_UINT16(&map[VAL3], val);
				c._gold += val;
				line = Common::String::format("+%d %s", val,
					STRING["maps.map16.gold"].c_str());
				break;

			case 3:
				val = 15 << map[VAL4];
				WRITE_LE_UINT16(&map[VAL3], val);
				c._gems += val;
				line = Common::String::format("+%d %s", val,
					STRING["maps.map16.gems"].c_str());
				break;

			default:
				line = STRING["maps.map16.loser"];
				break;
			}
		}

		_results.push_back(line);
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
