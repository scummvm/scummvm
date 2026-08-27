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

#include "ultima/ultima1/views/main_menu.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_EDGE = 15; // white - used for the title/tagline
constexpr int COLOR_TEXT = 11; // light cyan - used for the menu options

MainMenu::MainMenu() : Dialog("MainMenu"), _cursor("Cursor", this) {
	// Position the cursor right after "Thy choice: "
	_cursor.setPosition(Common::Point(25, 18));
}

void MainMenu::draw() {
	auto s = getSurface();
	s.clear();
	drawFrame();

	s.setColor(COLOR_EDGE);
	s.writeString(Common::Point(12, 6), "*** Ultima I ***");
	s.writeString(Common::Point(14, 9), "from darkest");
	s.writeString(Common::Point(14, 10), "dungeons, to");
	s.writeString(Common::Point(13, 11), "deepest space!");

	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(8, 14), "a) Generate new character");
	s.writeString(Common::Point(8, 15), "b) Continue previous game");
	s.writeString(Common::Point(13, 18), "Thy choice: ");
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	switch (toupper(msg.ascii)) {
	case 'A':
		replaceView("CreateCharacter");
		break;

	case 'B':
		g_engine->loadGameDialog();
		break;

	default:
		break;
	}

	return true;
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
