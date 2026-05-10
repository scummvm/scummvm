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

#include "ultima/ultima0/views/create_character.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

bool CreateCharacter::msgFocus(const FocusMessage &msg) {
	g_engine->_player.init();
	_mode = LUCKY_NUMBER;
	_input = "";
	return true;
}

void CreateCharacter::draw() {
	auto s = getSurface();
	const auto &player = g_engine->_player;
	s.clear();

	// Lucky number
	s.writeString(Common::Point(1, 4), "Type thy Lucky Number.....");
	if (_mode == LUCKY_NUMBER)
		s.writeString(_input);
	else
		s.writeString(Common::String::format("%d", player._luckyNumber));

	// Level of play
	if (_mode >= LEVEL) {
		s.writeString(Common::Point(1, 6), "Level of play (1-10)......");
		if (_mode == LEVEL)
			s.writeString(_input);
		else
			s.writeString(Common::String::format("%d", player._skill));
	}

	// Stats
	if (_mode >= STATS) {
		for (int i = 0; i < MAX_ATTR; ++i) {
			Common::String line = ATTRIB_NAMES[i];
			while (line.size() < 15)
				line += ".";
			line += Common::String::format("%d", player._attr[i]);
			s.writeString(Common::Point(1, 9 + i), line);
		}

		s.writeString(Common::Point(0, 16), "Shallt thou play with these qualities?");
	}

	// Class selection
	if (_mode == CLASS) {
		s.writeString(Common::Point(0, 18), "And shalt thou be a Fighter or a Mage?");
	}
}

bool CreateCharacter::msgKeypress(const KeypressMessage &msg) {
	auto &player = g_engine->_player;

	if (_mode == LUCKY_NUMBER || _mode == LEVEL) {
		if (Common::isDigit(msg.ascii) && _input.size() < 6) {
			_input += msg.ascii;
			redraw();
		} else if (msg.keycode == Common::KEYCODE_BACKSPACE && _input.size() > 0) {
			_input.deleteLastChar();
			redraw();
		} else if (msg.keycode == Common::KEYCODE_RETURN && !_input.empty()) {
			if (_mode == LUCKY_NUMBER) {
				player._luckyNumber = atoi(_input.c_str());
				_input.clear();
				_mode = LEVEL;
				redraw();
			} else {
				player._skill = atoi(_input.c_str());
				if (player._skill >= 1 && player._skill <= 10) {
					_input.clear();
					_mode = STATS;
					player.rollAttributes();
					redraw();
				}
			}
		}
	} else if (msg.keycode == Common::KEYCODE_y) {
		_mode = CLASS;
		redraw();
	} else if (msg.keycode == Common::KEYCODE_n) {
		player.rollAttributes();
		redraw();
	} else if (_mode == CLASS && (msg.keycode == Common::KEYCODE_f ||
		msg.keycode == Common::KEYCODE_m)) {
		player._class = toupper(msg.ascii);
		characterDone();
	}

	return true;
}

bool CreateCharacter::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		replaceView("Title");
		return true;
	}

	return true;
}

void CreateCharacter::characterDone() {
	// Generate the world map
	g_engine->_worldMap.init(g_engine->_player);

	// Enter the town
	replaceView("Town");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
