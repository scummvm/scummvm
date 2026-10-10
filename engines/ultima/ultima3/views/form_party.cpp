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

#include "ultima/ultima3/views/form_party.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

FormParty::FormParty() : WindowView("FormParty") {
	for (int i = 0; i < Data::PARTY_MAX; ++i)
		_numbers[i].setup(24, 16 + i);
}

bool FormParty::msgFocus(const FocusMessage &msg) {
	for (int i = 0; i < Data::PARTY_MAX; ++i)
		_numbers[i].reset();

	if (_G(savegame)._partySize != 0) {
		_showSelection = false;
		showMessage("(Party in Use)", 13, 16);
	} else {
		_showSelection = true;
		_state = ENTRY;
	}

	return View::msgFocus(msg);
}

void FormParty::showMessage(const char *message, int col, int row) {
	_state = MESSAGE;
	_message = message;
	_messageCol = col;
	_messageRow = row;
}

void FormParty::fail(const char *message) {
	_G(savegame).clearPartySelection();
	showMessage(message, 13, 21);
}

void FormParty::finish() {
	if (_G(savegame)._partySize == 0) {
		fail("(No one there)");
	} else {
		// A party that came to grief in a town starts out again on the world map
		if (_G(savegame)._mapLoaded && (_G(savegame)._location == Data::LOCATION_TOWN ||
				_G(savegame)._location == Data::LOCATION_CASTLE))
			_G(map) = _G(worldMap);

		_G(savegame).formParty();
		showMessage("(Formed)", 16, 21);
	}
}

void FormParty::draw() {
	auto s = getSurface();
	clearWindow(s);

	s.writeString(Common::Point(13, 13), "Form the Party");

	if (_showSelection) {
		for (int i = 0; i < Data::PARTY_MAX; ++i) {
			s.writeString(Common::Point(15, 16 + i), Common::String::format("Player-%d-", i + 1));
			_numbers[i].draw(s, _state == ENTRY && i == _G(savegame)._partySize);
		}
	}

	if (_state == MESSAGE) {
		s.writeString(Common::Point(_messageCol, _messageRow), _message);
		drawSpacePrompt(s);
	}
}

bool FormParty::msgKeypress(const KeypressMessage &msg) {
	if (_state == MESSAGE) {
		if (isSpaceKey(msg))
			close();
		return true;
	}

	Data::Savegame &save = _G(savegame);
	NumberInput &input = _numbers[save._partySize];

	if (input.handleKey(msg)) {
		int number = input.decimal();

		if (number > Data::ROSTER_COUNT) {
			fail("(No one there)");
		} else if (number == 0) {
			finish();
		} else if (save.entry(number).isEmpty()) {
			fail("(No one there)");
		} else if (save.entry(number).isInParty()) {
			fail("Already Selected");
		} else {
			save.addToParty(number);

			if (save._partySize == Data::PARTY_MAX)
				finish();
		}
	}

	redraw();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
