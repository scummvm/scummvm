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

#include "mm/mm1/views/maps/Child.h"
#include "mm/mm1/maps/map34.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

Child::Child() : TextView("Child") {
	_bounds = getLineBounds(17, 24);
}

void Child::draw() {
	clearSurface();
	writeString(0, 0, STRING["maps.map34.child"]);
	writeString(0, 3, STRING["maps.map34.options1"]);
	writeString(10, 4, STRING["maps.map34.options2"]);
	writeString(10, 5, STRING["maps.map34.options3"]);
}

bool Child::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode < Common::KEYCODE_1 || msg.keycode > Common::KEYCODE_3)
		return true;

	Common::String line;
	int align;
	switch (msg.keycode) {
	case Common::KEYCODE_1:
		line = STRING["maps.map34.prisoner_flees"];
		align = GOOD;
		break;

	case Common::KEYCODE_2:
		line = STRING["maps.map34.prisoner_cowers"];
		align = EVIL;
		break;

	default:
		align = NEUTRAL;
		break;
	}

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if (!(c._flags[1] & CHARFLAG1_4)) {
			c._flags[1] |= CHARFLAG1_4;
			if (align == c._alignment)
				c._v6e += 32;
		}
	}

	findView("GameParty")->redraw();
	if (align != NEUTRAL) {
		clearSurface();
		writeString(0, 1, line);
		Sound::sound(SOUND_2);
	}

	close();
	return true;
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
