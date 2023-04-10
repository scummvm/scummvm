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

#include "mm/mm1/views_enh/interactions/alien.h"
#include "mm/mm1/maps/map31.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Alien::Alien() : Interaction("Alien", 37) {
	_title = STRING["maps.emap31.alien_title"];
	addText(STRING["maps.emap31.alien"]);
	addButton(STRING["maps.emap31.option_a"], 'A');
	addButton(STRING["maps.emap31.option_b"], 'B');
	addButton(STRING["maps.emap31.option_c"], 'C');
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
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
