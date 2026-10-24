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
#include "mm/mm1/data/party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Trade::Trade() : ScrollView("Trade") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

bool Trade::msgFocus(const FocusMessage &msg) {
	_btnIcons.load("view.icn");

	if (dynamic_cast<TextEntry *>(msg._priorView) == nullptr) {
		_shareMode = false;
		setMode(TRADE_OPTIONS);
	}

	return ScrollView::msgFocus(msg);
}

bool Trade::msgUnfocus(const UnfocusMessage &msg) {
	_btnIcons.clear();
	return ScrollView::msgUnfocus(msg);
}

bool Trade::msgGame(const GameMessage &msg) {
	if (msg._name == "SHARE") {
		_shareMode = true;
		setMode(TRADE_OPTIONS);
		redraw();
		return true;
	}

	return ScrollView::msgGame(msg);
}

void Trade::draw() {
	ScrollView::draw();

	switch (_mode) {
	case TRADE_OPTIONS:
		drawOptions();
		break;
	default:
		writeString(0, 5, "How much:");
		break;
	}
}

bool Trade::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case TRADE_OPTIONS:
		switch (msg.keycode) {
		case Common::KEYCODE_m:
			if (_shareMode) {
				Party::share(TK_GEMS);
				close();
			} else {
				setMode(TRADE_GEMS);
			}
			break;
		case Common::KEYCODE_g:
			if (_shareMode) {
				Party::share(TK_GOLD);
				close();
			} else {
				setMode(TRADE_GOLD);
			}
			break;
		case Common::KEYCODE_f:
			if (_shareMode) {
				Party::share(TK_FOOD);
				close();
			} else {
				setMode(TRADE_FOOD);
			}
			break;
		case Common::KEYCODE_i:
			if (!_shareMode) {
				close();
				send(InfoMessage("To trade items, select an item from the inventory, and then select the portrait of the "
					"character to trade to"));
			}
			break;
		default:
			break;
		}

	default:
		break;
	}

	return true;
}

bool Trade::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

void Trade::abortFunc() {
	g_events->close();
}

void Trade::enterFunc(const Common::String &str) {
	Trade *view = static_cast<Trade *>(g_events->focusedView());
	int amount = atoi(str.c_str());
	view->amountEntered(amount);
}

void Trade::setMode(TradeMode mode) {
	_mode = mode;

	clearButtons();

	switch (_mode) {
	case TRADE_OPTIONS:
		addButton(&_btnIcons, Common::Point(80, 0), 32, Common::KEYCODE_g);
		addButton(&_btnIcons, Common::Point(158, 0), 34, Common::KEYCODE_m);
		addButton(&_btnIcons, Common::Point(80, 20), 36, Common::KEYCODE_f);
		if (!_shareMode)
			addButton(&_btnIcons, Common::Point(158, 20), 26, Common::KEYCODE_i);
		addButton(&g_globals->_escSprites, Common::Point(0, 20), 0, KEYBIND_ESCAPE);
		break;

	default:
		// Gold is stored as a 24-bit value (maximum 16,777,215),
		// so gold transfers need an 8-digit entry field.
		draw();
		_textEntry.display(70, 157, _mode == TRADE_GOLD ? 8 : 5, true,
			abortFunc, enterFunc);
	}
}

void Trade::drawOptions() {
	writeString(0, 5, _shareMode ? "share all:" :
		"Trade which:");
	writeString(28, 25, "Exit");
	writeString(108, 5, "\x01""37Gold");
	writeString(186, 5, "Ge\x01""37ms");
	writeString(108, 25, "\x01""37Food");
	if (!_shareMode)
		writeString(186, 25, "\x01""37Items");
}

void Trade::amountEntered(uint amount) {
	if (_shareMode)
		return;

	close();
	send("CharacterInventory", GameMessage("TRADE", TRADE_NAMES[_mode], amount));
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
