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

#include "mm/mm1/views/interactions/lord_archer.h"
#include "mm/mm1/maps/map40.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

LordArcher::LordArcher() : TextView("LordArcher") {
	_bounds = getLineBounds(20, 24);
}

bool LordArcher::msgFocus(const FocusMessage &msg) {
	Sound::sound(SOUND_3);
	return TextView::msgFocus(msg);
}

void LordArcher::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map40.archer"]);
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
} // namespace Views
} // namespace MM1
} // namespace MM
