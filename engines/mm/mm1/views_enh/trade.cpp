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

#include "mm/mm1/views_enh/trade.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Trade::Trade() : PartyView("Trade") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

bool Trade::msgFocus(const FocusMessage &msg) {
	setMode(TRADE_OPTIONS);
	_btnIcons.load("view.icn");

	return PartyView::msgFocus(msg);
}

void Trade::draw() {
	PartyView::draw();
	_btnIcons.clear();

	switch (_mode) {
	case TRADE_OPTIONS:
		drawOptions();
		break;
	}
}

bool Trade::msgKeypress(const KeypressMessage &msg) {

	return true;
}

void Trade::setMode(TradeMode mode) {
	_mode = mode;

	clearButtons();

	switch (_mode) {
	case TRADE_OPTIONS:
		addButton(&_btnIcons, Common::Point(80, 0), 32, Common::KEYCODE_g);
		addButton(&_btnIcons, Common::Point(158, 0), 34, Common::KEYCODE_d);
		addButton(&_btnIcons, Common::Point(80, 20), 36, Common::KEYCODE_f);
		addButton(&_btnIcons, Common::Point(158, 20), 38, Common::KEYCODE_i);
		addButton(&g_globals->_escSprites, Common::Point(0, 20), 0, KEYBIND_ESCAPE);
		break;
	}
}

void Trade::drawOptions() {
	writeString(0, 5, STRING["enhdialogs.trade.which"]);
	writeString(28, 25, STRING["enhdialogs.misc.exit"]);
	writeString(108, 5, STRING["enhdialogs.trade.gold"]);
	writeString(186, 5, STRING["enhdialogs.trade.gems"]);
	writeString(108, 25, STRING["enhdialogs.trade.food"]);
	writeString(186, 25, STRING["enhdialogs.trade.items"]);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
