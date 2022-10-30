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

bool Search::msgGame(const GameMessage &msg) {
	if (msg._name != "SHOW")
		return false;

	bool hasStuff = g_globals->_treasure.present();
	if (hasStuff) {
		// Focus view to show what was found
		open();

	} else {
		// Otherwise send an info message to say nothing was found
		Common::String line = STRING["dialogs.search.search"] +
			STRING["dialogs.search.nothing"];
		send(InfoMessage(0, 1, line));
	}

	return true;
}

bool Search::msgFocus(const FocusMessage &msg) {
	_mode = INITIAL;
	return true;
}

void Search::draw() {
	Common::String line;
	clearSurface();

	switch (_mode) {
	case INITIAL:
		Sound::sound(SOUND_2);
		line = STRING["dialogs.search.search"] +
			STRING["dialogs.search.you_found"];
		writeString(0, 1, line);
		delaySeconds(3);
		break;

	case OPTIONS:
		writeString(1, 1, STRING["dialogs.search.options1"]);
		writeString(20, 2, STRING["dialogs.search.options2"]);
		writeString(20, 3, STRING["dialogs.search.options3"]);
		escToGoBack(0, 3);
		break;
	}
}

bool Search::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case INITIAL:
		cancelDelay();
		timeout();
		break;

	case OPTIONS:
		switch (msg.keycode) {
		case Common::KEYCODE_ESCAPE:
			close();
			break;
		case Common::KEYCODE_1:
			// TODO: 1, 2, 3
			break;
		}
		break;
	}

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

	_mode = OPTIONS;
	draw();
}

} // namespace Views
} // namespace MM1
} // namespace MM
