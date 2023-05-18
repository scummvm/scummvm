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

#include "mm/mm1/views_enh/interactions/lord_archer.h"
#include "mm/mm1/maps/map40.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

LordArcher::LordArcher() : Interaction("LordArcher", 33) {
	_title = STRING["maps.emap40.title"];
	addText(STRING["maps.emap40.archer"]);
	addButton(STRING["maps.map40.yes"], 'Y');
	addButton(STRING["maps.map40.no"], 'N');
}

bool LordArcher::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_y || msg.keycode == Common::KEYCODE_n) {
		MM1::Maps::Map40 &map = *static_cast<MM1::Maps::Map40 *>(g_maps->_currentMap);
		close();

		if (msg.keycode == Common::KEYCODE_y) {
			map.archerSubmit();
		} else {
			map.archerResist();
		}
	}

	return true;
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
