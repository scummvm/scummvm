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

#include "ultima/ultima3/views/main_menu.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

// Escape (the last key) acts as Return
static const char OPTION_KEYS[] = "ROJ\x1B";
static const char *const OPTION_WORDS[] = { "Return", "Organize", "Journey", "Return" };

MainMenu::MainMenu() : WindowView("MainMenu") {
	_choice.setup(OPTION_KEYS, OPTION_WORDS, 4, 23, 15);
}

bool MainMenu::msgFocus(const FocusMessage &msg) {
	_choice.reset();
	return View::msgFocus(msg);
}

void MainMenu::draw() {
	auto s = getSurface();

	clearWindow(s);
	drawBorder(s);

	s.writeString(Common::Point(3, 21), "(C)-1983 By James R. Van Artsdalen");
	s.writeString(Common::Point(13, 22), "and Lord British");
	s.writeString(Common::Point(7, 11), "From the depths of hell...");
	s.writeString(Common::Point(7, 12), "...he comes for VENGEANCE!");
	s.writeString(Common::Point(16, 15), "Option: ");
	s.writeString(Common::Point(11, 17), "Return to the View");
	s.writeString(Common::Point(12, 18), "Organize a Party");
	s.writeString(Common::Point(13, 19), "Journey Onward");

	_choice.draw(s, true);
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	if (_choice.handleKey(msg)) {
		switch (_choice.key()) {
		case 'O':
			addView("PartyMenu");
			break;
		case 'J':
			// Setting out is wired up once the world engine lands
			if (_G(savegame)._partySize == 0 || !_G(savegame).hasLivingPartyMember())
				addView("JourneyOnward");
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
