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

#include "ultima/ultima3/views/party_menu.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

// Q and Escape also go back to the main menu
static const char OPTION_KEYS[] = "ECFDTMQL\x1B";
static const char *const OPTION_WORDS[] = {
	"Examine", "Create", "Form", "Disperse", "Terminate", "Main", "Main", "Look", "Main"
};

PartyMenu::PartyMenu() : WindowView("PartyMenu") {
	_choice.setup(OPTION_KEYS, OPTION_WORDS, 9, 23, 13);
}

bool PartyMenu::msgFocus(const FocusMessage &msg) {
	_choice.reset();
	return View::msgFocus(msg);
}

void PartyMenu::draw() {
	auto s = getSurface();

	clearWindow(s);
	drawBorder(s);

	s.writeString(Common::Point(11, 11), "Party Organization");
	s.writeString(Common::Point(16, 13), "Option:");
	s.writeString(Common::Point(10, 15), "Examine the Register");
	s.writeString(Common::Point(11, 16), "Create a Character");
	s.writeString(Common::Point(13, 17), "Form the Party");
	s.writeString(Common::Point(11, 18), "Disperse the Party");
	s.writeString(Common::Point(10, 19), "Terminate a Character");
	s.writeString(Common::Point(11, 20), "Look at a Character");
	s.writeString(Common::Point(15, 21), "Main Menu");

	_choice.draw(s, true);
}

bool PartyMenu::msgKeypress(const KeypressMessage &msg) {
	if (_choice.handleKey(msg)) {
		switch (_choice.key()) {
		case 'E':
			addView("Register");
			break;
		case 'C':
			addView("CreateCharacter");
			break;
		case 'F':
			addView("FormParty");
			break;
		case 'D':
			addView("DisperseParty");
			break;
		case 'T':
			addView("TerminateCharacter");
			break;
		case 'L':
			addView("CharacterDetails");
			break;
		default:
			close();
			return true;
		}
	}

	redraw();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
