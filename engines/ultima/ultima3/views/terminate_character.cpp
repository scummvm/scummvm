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

#include "ultima/ultima3/views/terminate_character.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

TerminateCharacter::TerminateCharacter() : WindowView("TerminateCharacter") {
	_entry.setup(22, 18);
}

bool TerminateCharacter::msgFocus(const FocusMessage &msg) {
	_message = nullptr;
	_entry.reset();
	return View::msgFocus(msg);
}

void TerminateCharacter::showMessage(const char *message, int col, int row) {
	_message = message;
	_messageCol = col;
	_messageRow = row;
}

void TerminateCharacter::draw() {
	auto s = getSurface();
	clearWindow(s);

	s.writeString(Common::Point(15, 15), "Terminate!");
	s.writeString(Common::Point(16, 18), "Entry#");
	_entry.draw(s, !_message);

	if (_message) {
		s.writeString(Common::Point(_messageCol, _messageRow), _message);
		drawSpacePrompt(s);
	}
}

bool TerminateCharacter::msgKeypress(const KeypressMessage &msg) {
	if (_message) {
		if (isSpaceKey(msg))
			close();
		return true;
	}

	if (_entry.handleKey(msg)) {
		int number = _entry.decimal();

		if (number < 1 || number > Data::ROSTER_COUNT) {
			showMessage("(1-20 Only!)", 14, 21);
		} else {
			Data::RosterEntry &entry = _G(savegame).entry(number);

			if (entry.isEmpty()) {
				showMessage("(No one there)", 13, 20);
			} else if (entry.isInParty()) {
				showMessage("(With a party)", 13, 21);
			} else {
				entry.clear();
				showMessage("(Terminated)", 14, 21);
			}
		}
	}

	redraw();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
