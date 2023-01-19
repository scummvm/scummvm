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

#include "mm/mm1/views/maps/hacker.h"
#include "mm/mm1/maps/map36.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

Hacker::Hacker() : TextView("Hacker") {
	_bounds = getLineBounds(20, 24);
}

bool Hacker::msgFocus(const FocusMessage &msg) {
	g_globals->_currCharacter = &g_globals->_party[0];
	_canAccept = !g_globals->_currCharacter->_quest;
	return TextView::msgFocus(msg);
}

void Hacker::draw() {
	MM1::Maps::Map36 &map = *static_cast<MM1::Maps::Map36 *>(g_maps->_currentMap);
	clearSurface();

	if (_canAccept) {
		send(SoundMessage(
			0, 1, STRING["maps.map36.hacker1"],
			0, 2, STRING["maps.map36.hacker2"]
		));
	} else {
		int questNum = g_globals->_party[0]._quest;
		Common::String line;

		if (questNum >= 15 || questNum <= 21)
			line = map.checkQuestComplete();
		else
			line = STRING["maps.map36.hacker4"];

		g_maps->_mapPos.x--;
		map.redrawGame();

		clearSurface();
		send(SoundMessage(
			0, 1, STRING["maps.map36.hacker1"],
			0, 2, line
		));
		close();
	}
}

bool Hacker::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map36 &map = *static_cast<MM1::Maps::Map36 *>(g_maps->_currentMap);

	if (_canAccept) {
		if (msg.keycode == Common::KEYCODE_y) {
			map.acceptQuest();

			clearSurface();
			writeString(0, 1, STRING["maps.map36.hacker1"]);
			writeString(0, 2, STRING[Common::String::format(
				"maps.map36.ingredients.%d",
				g_globals->_party[0]._quest)]);
			close();

		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
			map.redrawGame();
		}
	}

	return true;
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
