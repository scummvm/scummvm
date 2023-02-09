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

#include "mm/mm1/views/rest.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/game/rest.h"

namespace MM {
namespace MM1 {
namespace Views {

Rest::Rest() : TextView("Rest") {
	_bounds = getLineBounds(21, 24);
}

bool Rest::msgGame(const GameMessage &msg) {
	if (msg._name == "REST") {
		// Show the dialog
		addView(this);
		return true;
	}

	return false;
}

bool Rest::msgFocus(const FocusMessage &msg) {
	_mode = CONFIRM;
	if (g_maps->_currentState & 8)
		tooDangerous();
	return true;
}

void Rest::draw() {
	if (_mode == CONFIRM) {
		clearSurface();
		writeString(0, 0, STRING["dialogs.game.rest.rest_here"]);
	}
}

bool Rest::msgKeypress(const KeypressMessage &msg) {
	if (_mode == CONFIRM) {
		if (msg.keycode == Common::KEYCODE_n) {
			close();
		} else if (msg.keycode == Common::KEYCODE_y) {
			close();
			Game::Rest::check();
		}
	}

	return true;
}

bool Rest::msgAction(const ActionMessage &msg) {
	if (_mode == CONFIRM) {
		if (msg._action == KEYBIND_ESCAPE) {
			close();
		} else if (msg._action == KEYBIND_SELECT) {
			close();
			Game::Rest::check();
		}
	}

	return true;
}

void Rest::tooDangerous() {
	_mode = TOO_DANGEROUS;
	clearSurface();
	writeString(2, 0, STRING["dialogs.game.rest.too_dangerous"]);
	delaySeconds(3);
}

void Rest::timeout() {
	close();
}

} // namespace Views
} // namespace MM1
} // namespace MM
