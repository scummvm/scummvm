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

#include "mm/mm1/views/character_info.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void CharacterInfo::draw() {
	assert(g_globals->_currCharacter);
	CharacterBase::draw();

	switch (_state) {
	case DISPLAY:
		writeString(0, 21, STRING["dialogs.character.legend1"]);
		writeString(0, 22, STRING["dialogs.character.legend2"]);
		writeString(0, 23, STRING["dialogs.character.legend3"]);
		writeString(0, 24, STRING["dialogs.character.legend4"]);
		break;

	case RENAME:
		break;

	case DELETE:
		break;
	}
}

bool CharacterInfo::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		if (_state != DISPLAY) {
			redraw();
		} else {
			close();
		}

		_state = DISPLAY;
		return true;
	}

	switch (_state) {
	case DISPLAY:
		if ((msg.flags & Common::KBD_CTRL) && msg.keycode == Common::KEYCODE_n) {
			_state = RENAME;
			_newName = "";
			redraw();
		} else if ((msg.flags & Common::KBD_CTRL) && msg.keycode == Common::KEYCODE_d) {
			_state = DELETE;
			redraw();
		}
		break;

	case RENAME:
		if (msg.ascii >= 32 && msg.ascii <= 127) {
			_newName += toupper(msg.ascii);
			redraw();
		}
		if (msg.keycode == Common::KEYCODE_RETURN || _newName.size() == 15) {
			strncpy(g_globals->_currCharacter->_name, _newName.c_str(), 16);
			_state = DISPLAY;
			redraw();
		} else if (msg.keycode == Common::KEYCODE_BACKSPACE &&
				!_newName.empty()) {
			_newName.deleteLastChar();
			redraw();
		}
		break;

	case DELETE:
		if (msg.keycode == Common::KEYCODE_y) {
			// Removes the character and returns to View All Characters
			g_globals->_roster.remove(g_globals->_currCharacter);
			close();
		} else {
			// Any other keypress returns to display mode
			redraw();
		}

		_state = DISPLAY;
		break;
	}

	return true;
}

bool CharacterInfo::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6 &&
			_state == DISPLAY) {
		g_globals->_currCharacter = &g_globals->_party[
			msg._action - KEYBIND_VIEW_PARTY1];
		addView();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
