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

#include "mm/mm1/views_enh/locations/blacksmith_items.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

BlacksmithItems::BlacksmithItems() : ItemsView("BlacksmithItems") {
	_btnSprites.load("buy.icn");
	addButton(0, STRING["enhdialogs.blacksmith.buttons.weapons"], Common::KEYCODE_w);
	addButton(2, STRING["enhdialogs.blacksmith.buttons.armor"], Common::KEYCODE_a);
	addButton(4, STRING["enhdialogs.blacksmith.buttons.accessories"], Common::KEYCODE_c);
	addButton(6, STRING["enhdialogs.blacksmith.buttons.misc"], Common::KEYCODE_m);
	addButton(8, STRING["enhdialogs.blacksmith.buttons.buy"], Common::KEYCODE_b);
	addButton(10, STRING["enhdialogs.blacksmith.buttons.sell"], Common::KEYCODE_s);
	addButton(12, STRING["enhdialogs.blacksmith.buttons.exit"], Common::KEYCODE_ESCAPE);
}

bool BlacksmithItems::msgFocus(const FocusMessage &msg) {
	ItemsView::msgFocus(msg);

	return true;
}

void BlacksmithItems::draw() {
	ItemsView::draw();

	//	writeString(0, 0, STRING["dialogs.game.BlacksmithItems.BlacksmithItems_here"], ALIGN_MIDDLE);
}

bool BlacksmithItems::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;
	/*
		if (_mode == CONFIRM) {
			if (msg.keycode == Common::KEYCODE_n) {
				close();
			} else if (msg.keycode == Common::KEYCODE_y) {
				close();
				Game::BlacksmithItems::check();
			}
		}
	*/
	return ItemsView::msgKeypress(msg);
}

bool BlacksmithItems::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;
	/*
	if (_mode == CONFIRM) {
		if (msg._action == KEYBIND_ESCAPE) {
			close();
		} else if (msg._action == KEYBIND_SELECT) {
			close();
			Game::BlacksmithItems::check();
		}
	}
	*/
	return ItemsView::msgAction(msg);
}

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
