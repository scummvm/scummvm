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

#include "mm/mm1/views_enh/items_view.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define BUTTON_WIDTH 35
#define EXIT_X 275

ItemsView::ItemsView(const Common::String &name) : PartyView(name),
		_buttonsArea(Common::Rect(0, 101, 320, 146)) {
	_bounds = Common::Rect(0, 0, 320, 146);
}

void ItemsView::addButton(int frame, const Common::String &text,
		Common::KeyCode keycode) {
	Common::Point pt(_btnText.size() * BUTTON_WIDTH + 5, 101);
	if (keycode == Common::KEYCODE_ESCAPE) {
		pt.x = EXIT_X;
		PartyView::addButton(&g_globals->_escSprites, pt, 0, KEYBIND_ESCAPE);
	} else {
		PartyView::addButton(&_btnSprites, pt, frame, keycode);
	}

	_btnText.push_back(text);
}

bool ItemsView::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	_selectedItem = -1;
	return true;
}

void ItemsView::draw() {
	// Draw the outer frame and buttons
	PartyView::draw();

	// Draw the frame surrounding the buttons area
	const Common::Rect r = _bounds;
	_bounds = _buttonsArea;
	frame();
	_bounds = r;

	// Draw button text
	setReduced(true);
	for (uint i = 0; i < _btnText.size(); ++i) {
		Common::Point pt(i * BUTTON_WIDTH + 5, 123);
		if (i == (_btnText.size() - 1))
			pt.x = EXIT_X;

		writeString(pt.x + 12, pt.y, _btnText[i], ALIGN_MIDDLE);
	}

	// List the items
	for (uint i = 0; i < _items.size(); ++i) {
		g_globals->_items.getItem(_items[i]);
		const Item &item = g_globals->_currItem;
		const Common::String line = Common::String::format(
			"%c) %s", _startingChar + i,
			item._name.c_str()
		);
		writeLine(2 + i, line, ALIGN_LEFT, 10);

		if (_costMode != NO_COST) {
			int cost = (_costMode == SHOW_COST) ? item._cost : item.getSellCost();
			writeLine(2 + i, Common::String::format("%d", cost),
				ALIGN_RIGHT);
		}
	}
	if (_items.size() == 0)
		writeLine(2, STRING["enhdialogs.misc.no_items"], ALIGN_LEFT, 10);
}

bool ItemsView::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	return PartyView::msgKeypress(msg);
}

bool ItemsView::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	} else {
		return PartyView::msgAction(msg);
	}
}

void ItemsView::timeout() {
	close();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
