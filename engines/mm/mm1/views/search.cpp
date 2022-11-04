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
		delaySeconds(2);
		break;

	case OPTIONS:
		writeString(1, 1, STRING["dialogs.search.options1"]);
		writeString(20, 2, STRING["dialogs.search.options2"]);
		writeString(20, 3, STRING["dialogs.search.options3"]);
		escToGoBack(0, 3);
		break;

	default:
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
			openContainer();
			break;
		case Common::KEYCODE_2:
			findRemoveTrap();
			break;
		case Common::KEYCODE_3:
			detectMagicTrap();
			break;
		default:
			break;
		}
		break;

	case RESPONSE:
		if (isDelayActive()) {
			cancelDelay();
			timeout();
		}
		break;

	case WHO_WILL_TRY:
		if (msg.keycode == Common::KEYCODE_ESCAPE) {
			_mode = OPTIONS;
			draw();

		} else if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= (Common::KEYCODE_0 + (int)g_globals->_party.size())) {
			// Character selected
			g_globals->_currCharacter = &g_globals->_party[
				msg.keycode - Common::KEYCODE_1];
			if ((g_globals->_currCharacter->_condition &
				(BAD_CONDITION | DEAD | STONE | ASLEEP)) != 0) {
				clearSurface();
				writeString(3, 2, STRING["dialogs.search.check_condition"]);
				delaySeconds(4);
			} else if (_removing) {
				findRemoveTrap2();
			} else {
				openContainer2();
			}
		}
		break;
	}

	return true;
}

void Search::timeout() {
	switch (_mode) {
	case INITIAL: {
		Maps::Map &map = *g_maps->_currentMap;
		_val1 = MIN(g_globals->_treasure._container * 8 +
			map[Maps::MAP_TRAP_THRESHOLD], 255);

		if (!g_globals->_treasure[1]) {
			g_globals->_treasure[1] = (getRandomNumber(100) < _val1) ? 1 : 2;
		}

		// Show the name of the container type in the game view
		send("View", HeaderMessage(
			STRING[Common::String::format("dialogs.search.containers.%d",
				g_globals->_treasure._container)]
		));

		// Display a graphic for the container type
		int gfxNum = g_globals->_treasure._container < WOODEN_BOX ? 4 : 2;
		send("View", DrawGraphicMessage(gfxNum + 65));

		_mode = OPTIONS;
		draw();
		break;
	}
	case RESPONSE:
		_mode = OPTIONS;
		draw();
		break;

	case WHO_WILL_TRY:
		draw();
		break;

	default:
		break;
	}
}

void Search::openContainer() {
	_removing = false;
	if (!whoWillTry())
		openContainer2();
}

void Search::openContainer2() {

}

void Search::findRemoveTrap() {
	_removing = true;
	if (!whoWillTry())
		findRemoveTrap2();
}

void Search::findRemoveTrap2() {
	Character &c = *g_globals->_currCharacter;

	if (g_globals->_treasure._trapType == 1) {
		byte val = c._trapCtr;
		if (getRandomNumber(val >= 100 ? val + 5 : 100) >= val) {
			// TODO
		}
	}
}

void Search::detectMagicTrap() {
	Character &c = *g_globals->_currCharacter;
	_mode = RESPONSE;

	if (c._class == PALADIN || c._class == CLERIC) {
		Sound::sound(SOUND_2);
		clearSurface();
		writeString(6, 2, STRING["dialogs.search.bad_class"]);
		delaySeconds(4);

	} else if (c._sp == 0) {
		Sound::sound(SOUND_2);
		clearSurface();
		writeString(6, 2, STRING["dialogs.search.no_sp"]);
		delaySeconds(4);

	} else {
		c._sp._current--;
		char magic = g_globals->_treasure[3] || g_globals->_treasure[4]
			|| g_globals->_treasure[5] || g_globals->_treasure[8] ? 'Y' : 'N';
		char trapped = g_globals->_treasure._trapType == 1 ? 'Y' : 'N';

		clearSurface();
		writeString(5, 1, Common::String::format(
			STRING["dialogs.search.magic_trap"].c_str(),
			magic, trapped));

		delaySeconds(8);
	}
}

bool Search::whoWillTry() {
	if (g_globals->_party.size() == 1) {
		g_globals->_currCharacter = &g_globals->_party[0];
		return true;
	} else {
		// Switch to mode to ask which character to use
		_mode = WHO_WILL_TRY;
		draw();
		return false;
	}
}

} // namespace Views
} // namespace MM1
} // namespace MM
