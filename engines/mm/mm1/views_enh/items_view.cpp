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
#include "mm/mm1/sound.h"

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
	PartyView::msgFocus(msg);

	// Disable the normal '1' to '6' character selection keybindings,
	// since we're using them in this dialog for item selection
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);

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
		if (isButtonEnabled(i)) {
			Common::Point pt(i * BUTTON_WIDTH + 5, 123);
			if (i == (_btnText.size() - 1))
				pt.x = EXIT_X;

			writeString(pt.x + 12, pt.y, _btnText[i], ALIGN_MIDDLE);
		}
	}

	// List the items
	for (int i = 0; i < (int)_items.size(); ++i) {
		g_globals->_items.getItem(_items[i]);
		const Item &item = g_globals->_currItem;
		const Common::String line = Common::String::format(
			"%d) %s", i + 1,
			item._name.c_str()
		);

		setTextColor(i == _selectedItem ? 15 : getLineColor());
		writeLine(2 + i, line, ALIGN_LEFT, 10);

		if (_costMode != NO_COST) {
			int cost = (_costMode == SHOW_COST) ? item._cost : item.getSellCost();
			writeLine(2 + i, Common::String::format("%d", cost),
				ALIGN_RIGHT);
		}
	}
	if (_items.size() == 0)
		writeLine(2, STRING["enhdialogs.misc.no_items"], ALIGN_LEFT, 10);
	setTextColor(0);
}

bool ItemsView::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= (int)(Common::KEYCODE_0 + _items.size())) {
		_selectedItem = msg.keycode - Common::KEYCODE_1;
		draw();

		itemSelected();
		return true;
	}

	return PartyView::msgKeypress(msg);
}

bool ItemsView::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._pos.x >= (_innerBounds.left + 10) &&
			msg._pos.x < _innerBounds.right) {
		int y = msg._pos.y - (_innerBounds.top + 2 * 9);

		if (y >= 0) {
			int lineNum = y / 9;
			if (lineNum < (int)_items.size()) {
				_selectedItem = lineNum;
				draw();

				itemSelected();
				return true;
			}
		}
	}

	return PartyView::msgMouseDown(msg);
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
	redraw();
}

void ItemsView::backpackFull() {
	displayMessage(STRING["dialogs.misc.backpack_full"]);
}

void ItemsView::notEnoughGold() {
	displayMessage(STRING["dialogs.misc.not_enough_gold"]);
}

void ItemsView::displayMessage(const Common::String &msg) {
	SoundMessage infoMsg(msg, ALIGN_MIDDLE);
	infoMsg._delaySeconds = 3;
	infoMsg._timeoutCallback = []() {
		ItemsView *view = static_cast<ItemsView *>(g_events->focusedView());
		view->timeout();
	};

	send(infoMsg);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
