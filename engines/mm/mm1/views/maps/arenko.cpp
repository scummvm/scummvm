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

#include "mm/mm1/views/maps/arenko.h"
#include "mm/mm1/maps/map28.h"
#include "mm/mm1/globals.h"

#define VAL1 63
#define VAL2 64

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

Arenko::Arenko() : TextView("Arenko") {
	_bounds = getLineBounds(20, 24);
}

bool Arenko::msgFocus(const FocusMessage &msg) {
	MM1::Maps::Map28 &map = *static_cast<MM1::Maps::Map28 *>(g_maps->_currentMap);

	if (!map[VAL1]) {
		_line = STRING["maps.map28.arenko"];
		map[VAL2] = 1;

	} else if (map[VAL1] < 19) {
		_line = STRING["maps.map28.keep_climbing"];

	} else {
		_line = STRING["maps.map28.well_done"];
	}

	return true;
}

void Arenko::draw() {
	clearSurface();
	writeString(0, 1, _line);
}

bool Arenko::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map28 &map = *static_cast<MM1::Maps::Map28 *>(g_maps->_currentMap);

	if (map[VAL1] == 19) {
		switch (msg.keycode) {
		case Common::KEYCODE_a:
			close();
			giveGold();
			return true;
		case Common::KEYCODE_b:
			close();
			giveGems();
			return true;
		case Common::KEYCODE_c:
			close();
			giveItem();
			return true;
		default:
			break;
		}
	} else {
		close();
	}

	return true;
}

bool Arenko::msgAction(const ActionMessage &msg) {
	MM1::Maps::Map28 &map = *static_cast<MM1::Maps::Map28 *>(g_maps->_currentMap);
	if (map[VAL1] != 19)
		close();

	return true;
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
