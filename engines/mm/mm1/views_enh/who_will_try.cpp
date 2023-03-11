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

#include "mm/mm1/views_enh/who_will_try.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WhoWillTry::WhoWillTry() : PartyView("WhoWillTry") {
	_bounds = Common::Rect(234, 144, 320, 200);
}

void WhoWillTry::display(WhoWillProc callback) {
	WhoWillTry *view = static_cast<WhoWillTry *>(g_events->findView("WhoWillTry"));
	view->open(callback);
}

void WhoWillTry::open(WhoWillProc callback) {
	_callback = callback;

	if (g_globals->_party.size() > 1) {
		// Select the character to use
		addView();

	} else {
		// With only one party member, they're automatically used
		callback(0);
	}
}

bool WhoWillTry::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		close();
		_callback(g_globals->_party.indexOf(g_globals->_currCharacter));
	}

	return true;
}

void WhoWillTry::draw() {
	PartyView::draw();

	writeString(0, 0, Common::String::format(
		STRING["dialogs.misc.who_will_try"].c_str(),
		'0' + g_globals->_party.size()));
}

bool WhoWillTry::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		_callback(-1);
		return true;
	} else if (msg._action >= KEYBIND_VIEW_PARTY1 &&
		msg._action <= KEYBIND_VIEW_PARTY6) {
		int charNum = msg._action - KEYBIND_VIEW_PARTY1;

		if (charNum < (int)g_globals->_party.size()) {
			close();
			_callback(charNum);
		}

		return true;
	}

	return PartyView::msgAction(msg);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
