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

#include "mm/mm1/views/spells/fly.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

void Fly::show(FlyCallback callback) {
	Fly *fly = dynamic_cast<Fly *>(g_events->findView("Fly"));
	assert(fly);

	fly->_callback = callback;
	fly->open();
}

Fly::Fly() : SpellView("Fly") {
	_bounds = getLineBounds(21, 24);
}

bool Fly::msgFocus(const FocusMessage &msg) {
	SpellView::msgFocus(msg);

	_mode = SELECT_X;
	_xIndex = _yIndex = 0;
	return 0;
}

void Fly::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(9, 0, STRING["dialogs.spells.fly_to_x"]);
	writeChar((_mode == SELECT_X) ? '_' : 'A' + _xIndex);

	if (_mode == SELECT_Y || _mode == CAST) {
		writeString(16, 1, STRING["dialogs.spells.fly_to_y"]);
		writeChar((_mode == SELECT_Y) ? '_' : '1' + _yIndex);
	}

	if (_mode == CAST) {
		writeString(24, 3, STRING["spells.enter_to_cast"]);
	}
}

bool Fly::msgKeypress(const KeypressMessage &msg) {
	if (_mode == SELECT_X && msg.keycode >= Common::KEYCODE_a
		&& msg.keycode <= Common::KEYCODE_d) {
		// X map selected
		_mode = SELECT_Y;
		_xIndex = msg.keycode - Common::KEYCODE_a;
		redraw();

	} else if (_mode == SELECT_Y && msg.keycode >= Common::KEYCODE_1
		&& msg.keycode <= Common::KEYCODE_4) {
		// Y map selected
		_mode = CAST;
		_yIndex = msg.keycode - Common::KEYCODE_1;
		redraw();

	} else if (_mode == CAST && msg.keycode == Common::KEYCODE_RETURN) {
		// Spell was cast
		close();
		int mapIndex = _yIndex * 5 + _xIndex;
		_callback(mapIndex);
	}

	return true;
}

bool Fly::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		_callback(-1);
		return true;
	}

	return false;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
