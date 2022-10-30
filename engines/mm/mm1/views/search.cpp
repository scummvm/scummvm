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

#include "mm/mm1/views/search.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Search::Search() : TextView("Search") {
	_bounds = getLineBounds(20, 24);
}

bool Search::msgFocus(const FocusMessage &msg) {
	_hasStuff = false;
	for (int i = 0; i < 6 && !_hasStuff; ++i)
		_hasStuff = g_globals->_treasure[i + 3] != 0;
	_mode = INITIAL;

	return true;
}

void Search::draw() {
	clearSurface();

	switch (_mode) {
	case INITIAL:
		if (!_hasStuff) {
			Common::String line = STRING["views.search.search"] +
				STRING["views.search.nothing"];
			writeString(0, 1, line);
			close();

		} else {
			Sound::sound(SOUND_2);
			Common::String line = STRING["views.search.search"] +
				STRING["views.search.you_found"];
			writeString(0, 1, line);
			delaySeconds(2);
		}
		break;

	case OPTIONS:
		break;
	}
}

bool Search::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

void Search::timeout() {
	Maps::Map &map = *g_maps->_currentMap;
	_val1 = MIN(g_globals->_treasure._container * 8 +
		map[Maps::MAP_48], 255);

	if (!g_globals->_treasure[1]) {
		g_globals->_treasure[1] = (getRandomNumber(100) < _val1) ? 1 : 2;
	}

	// Display a graphic for the container type
	int gfxNum = g_globals->_treasure._container < WOODEN_BOX ? 4 : 2;
	send("GameView", DrawGraphicMessage(gfxNum));

	// Show the name of the container type in the game view
	send("GameView", GameMessage(
		STRING[Common::String::format("views.search.containers.%d",
			g_globals->_treasure._container)]
	));
}

} // namespace Views
} // namespace MM1
} // namespace MM
