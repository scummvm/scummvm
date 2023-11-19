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

#include "mm/mm1/views/interactions/ghost.h"
#include "mm/mm1/maps/map37.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Ghost::Ghost() : TextView("Ghost") {
	_bounds = getLineBounds(20, 24);
}

void Ghost::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map37.okrim1"]);
}

bool Ghost::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_y || msg.keycode == Common::KEYCODE_n) {
		MM1::Maps::Map37 &map = *static_cast<MM1::Maps::Map37 *>(g_maps->_currentMap);

		if (msg.keycode == Common::KEYCODE_y) {
			g_globals->_party[0]._condition = ERADICATED;
		} else {
			clearSurface();
			writeString(0, 1, STRING["maps.map37.okrim2"]);

			map[MM1::Maps::MAP_29] = 32;
			map[MM1::Maps::MAP_47] = 8;
		}

		// Note: You get the ring whether or not you agree to it.
		// This is indeed how the original's logic is implemented
		close();
		g_globals->_treasure._items[2] = RING_OF_OKRIM_ID;
		g_events->addAction(KEYBIND_SEARCH);
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
