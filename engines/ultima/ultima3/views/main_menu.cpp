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

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int COLOR_TEXT = 3; // white, within CGA_PALETTE1

void MainMenu::draw() {
	auto s = getSurface();
	s.clear();
	s.setColor(COLOR_TEXT);

	s.writeString(Common::Point(11, 8), "R - Return to the View");
	s.writeString(Common::Point(11, 10), "O - Organize a Party");
	s.writeString(Common::Point(11, 12), "J - Journey Onward");
}

bool MainMenu::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_r:
	case Common::KEYCODE_o:
	case Common::KEYCODE_j:
		// Wired up once the roster/party data model and world engine exist
		break;
	default:
		break;
	}

	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
