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

#include "ultima/ultima2/views/title.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

bool Title::msgFocus(const FocusMessage &msg) {
	_cursor.show();
	return true;
}

void Title::draw() {
	auto s = getSurface();
	s.clear();

	s.writeString(Common::Point(15, 1),  "ULTIMA ][");
	s.writeString(Common::Point(16, 3),  "REVENGE");
	s.writeString(Common::Point(16, 5),  "OF  THE");
	s.writeString(Common::Point(14, 7),  "ENCHANTRESS");
	s.writeString(Common::Point(6, 22),  "(C)-1983,1989 BY LORD BRITISH");
	s.writeString(Common::Point(15, 23), "AND ORIGIN");
	s.writeString(Common::Point(5, 11), "TYPE - ");
	s.writeString(Common::Point(6, 13), "`D` - FOR A DEMONSTRATION");
	s.writeString(Common::Point(6, 15), "`P` - PLAY A GAME OF ULTIMA ][");
	s.writeString(Common::Point(6, 17), "`C` - CREATE A NEW CHARACTER");
	s.writeString(Common::Point(5, 19), "CHOICE:");
}

bool Title::msgGame(const GameMessage &msg) {
	char key = msg._stringValue.empty() ? 0 : toupper(msg._stringValue.firstChar());

	switch (key) {
	case 'D':
		replaceView("Demo");
		return true;
	case 'C':
		replaceView("CreateCharacter");
		return true;
	case 'P':
		if (!g_engine->savegamesExist()) {
			replaceView("CreateCharacter");
		} else {
			g_engine->loadGameDialog();
		}
		break;

	default:
		break;
	}

	return false;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
