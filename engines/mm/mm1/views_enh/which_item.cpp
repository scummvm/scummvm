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

#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WhichItem::WhichItem() : ScrollView("WhichItem") {
	_bounds = Common::Rect(50, 103, 266, 139);
	addButton(&g_globals->_escSprites, Common::Point(70, 0), 0, KEYBIND_ESCAPE);
}

void WhichItem::show(const Common::String &msg, KeyCallback callback) {
	WhichItem *view = static_cast<WhichItem *>(g_events->findView("WhichItem"));
	view->_msg = msg;
	view->_callback = callback;
	view->addView();
}

void WhichItem::draw() {
	ScrollView::draw();
	writeString(0, 0, _msg);
}

bool WhichItem::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_6) {
		close();
		_callback(Common::KeyState(msg.keycode));
		return true;
	} else {
		return false;
	}
}

bool WhichItem::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		_callback(Common::KeyState(Common::KEYCODE_ESCAPE));
		return true;
	} else {
		return false;
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
