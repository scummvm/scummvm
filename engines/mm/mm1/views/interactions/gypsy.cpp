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

#include "mm/mm1/views/interactions/gypsy.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

#define VAL1 123
#define ANSWER_OFFSET 167

Gypsy::Gypsy() : TextView("Gypsy") {
	_bounds = getLineBounds(20, 24);
}

bool Gypsy::msgFocus(const FocusMessage &msg) {
	TextView::msgFocus(msg);
	modeChanged(true);
	return true;
}

bool Gypsy::msgUnfocus(const UnfocusMessage &msg) {
	TextView::msgUnfocus(msg);
	modeChanged(false);
	return true;
}

void Gypsy::charSelected(uint charIndex) {
	if (charIndex < g_globals->_party.size()) {
		_character = &g_globals->_party[charIndex];

		if (!(_character->_flags[4] & CHARFLAG4_ASSIGNED)) {
			_character->_flags[4] = CHARFLAG4_ASSIGNED |
				(getRandomNumber(8) - 1);
		}

		redraw();
	}
}

void Gypsy::modeChanged(bool allowSelection) {
	if (allowSelection)
		MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_PARTY_MENUS);
	else
		MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);

}

void Gypsy::draw() {
	clearSurface();
	if (!_character) {
		writeString(0, 1, STRING["maps.map23.gypsy"]);

	} else {
		Common::String line = Common::String::format(
			STRING["maps.map23.your_sign_is"].c_str(),
			STRING[Common::String::format("colors.%d",
				_character->_flags[4] & CHARFLAG4_SIGN)].c_str()
		);

		writeString(0, 1, line);
	}
}

bool Gypsy::msgKeypress(const KeypressMessage &msg) {
	if (_character) {
		_character = nullptr;
		modeChanged(true);
		draw();

	} else {
		close();
	}

	return true;
}

bool Gypsy::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6) {
		uint idx = msg._action - KEYBIND_VIEW_PARTY1;
		charSelected(idx);
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
