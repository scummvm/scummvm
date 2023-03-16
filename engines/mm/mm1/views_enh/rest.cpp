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

#include "mm/mm1/views_enh/rest.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/game/rest.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Rest::Rest() : YesNo("Rest") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

bool Rest::msgFocus(const FocusMessage &msg) {
	if (g_maps->_currentState & 8)
		tooDangerous();
	else
		confirmRest();

	return true;
}

void Rest::draw() {
	YesNo::draw();

	if (_mode == CONFIRM) {
		writeString(0, 0, STRING["dialogs.game.rest.rest_here"], ALIGN_MIDDLE);
	} else {
		writeString(0, 0, STRING["enhdialogs.rest.too_dangerous"], ALIGN_MIDDLE);
	}
}

bool Rest::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

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
	if (endDelay())
		return true;

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
	closeYesNo();
	delaySeconds(3);
}

void Rest::confirmRest() {
	_mode = CONFIRM;
	openYesNo();
}

void Rest::timeout() {
	close();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
