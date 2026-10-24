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

#include "mm/mm1/views/main_menu.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {

void MainMenu::draw() {
	drawTextBorder();
	writeString(12, 4, "Might and Magic");
	writeString(6, 6, "Secret of the Inner Sanctum");
	writeString(15, 9, "Options");
	writeString(15, 10, "-------");

	writeString(5, 12, "'C'.......Create New Characters");
	writeString(5, 14, "'V'.......View All Characters");
	writeString(5, 16, "'#'.......Go to Town");
	writeString(4, 22, "Copr. 1986,1987-Jon Van Caneghem");
	writeString(10, 24, "All Rights Reserved");
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_c:
		addView("CreateCharacters");
		break;

	case Common::KEYCODE_v:
		addView("Characters");
		break;

	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
		g_globals->_startingTown = (Maps::TownId)(
			msg.keycode - Common::KEYCODE_0);
		replaceView("Inn");
		break;

	default:
		break;
	}

	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
