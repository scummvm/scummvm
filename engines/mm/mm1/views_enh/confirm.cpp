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

#include "mm/mm1/views_enh/confirm.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Confirm::Confirm() : ScrollView("Confirm") {
	_bounds = Common::Rect(99, 59, 237, 141);
	_bounds.setBorderSize(10);
	addButton(&g_globals->_confirmIcons, Common::Point(20, 44), 0, Common::KEYCODE_y);
	addButton(&g_globals->_confirmIcons, Common::Point(70, 44), 2, Common::KEYCODE_n);
}

void Confirm::show(const Common::String &msg,
		YNCallback callback) {
	Confirm *view = static_cast<Confirm *>(g_events->findView("Confirm"));
	view->_msg = msg;
	view->_callback = callback;

	view->addView();
}


void Confirm::draw() {
	ScrollView::draw();
	writeString(0, 0, _msg);
}

bool Confirm::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_y:
		return msgAction(ActionMessage(KEYBIND_SELECT));
		break;
	case Common::KEYCODE_n:
		return msgAction(ActionMessage(KEYBIND_ESCAPE));
		break;
	default:
		return true;
	}
}

bool Confirm::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_SELECT:
		close();
		_callback();
		break;
	case KEYBIND_ESCAPE:
		close();
		break;

	default:
		break;
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
