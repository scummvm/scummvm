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
#define EXIT_X 270

ItemsView::ItemsView(const Common::String &name) : PartyView(name),
		_buttonsArea(Common::Rect(0, 101, 320, 146)) {
	_bounds = Common::Rect(0, 0, 320, 146);
}

void ItemsView::addButton(int frame, const Common::String &text,
		Common::KeyCode keycode) {
	Common::Point pt(_btnText.size() * BUTTON_WIDTH + 5, 0);
	if (keycode == Common::KEYCODE_ESCAPE)
		pt.x = EXIT_X;

	PartyView::addButton(&_btnSprites, pt, frame, keycode);
	_btnText.push_back(text);
}

bool ItemsView::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);
	_selectedItem = -1;
	return true;
}

void ItemsView::draw() {
	// Manually draw a frame for the entire area to avoid
	// also drawing the buttons
	frame();
	fill();

	// Now draw the buttons area
	const Common::Rect r = _bounds;
	_bounds = _buttonsArea;
	PartyView::draw();
	_bounds = r;

	// Draw button text
	setReduced(true);
	for (uint i = 0; i < _btnText.size(); ++i) {
		Common::Point pt(i * BUTTON_WIDTH + 5, 122);
		if (i == (_btnText.size() - 1))
			pt.x = EXIT_X;

		writeString(pt.x + 18, pt.y, _btnText[i], ALIGN_MIDDLE);
	}

	// TODO: drawing items
}

bool ItemsView::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	return true;
}

bool ItemsView::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return true;
}

void ItemsView::timeout() {
	close();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
