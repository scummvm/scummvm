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

#include "mm/mm1/views/interactions/alien.h"
#include "mm/mm1/maps/map31.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

Alien::Alien() : TextView("Alien") {
	_bounds = getLineBounds(20, 24);
}

void Alien::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map31.alien"]);
}

bool Alien::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map31 &map = *static_cast<MM1::Maps::Map31 *>(
		g_maps->_currentMap);

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		close();
		map.hostile();
		break;
	case Common::KEYCODE_b:
		close();
		map.neutral();
		break;
	case Common::KEYCODE_c:
		close();
		map.friendly();
		break;
	default:
		break;
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
