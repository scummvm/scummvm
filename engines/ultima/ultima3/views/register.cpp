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

#include "ultima/ultima3/views/register.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

void Register::draw() {
	auto s = getSurface();
	clearWindow(s);

	s.writeString(Common::Point(16, 11), "Register");

	for (int number = 1; number <= Data::ROSTER_COUNT; ++number) {
		// The first ten fill the left column and the rest the right
		int col = number <= 10 ? 1 : 20;
		int row = number <= 10 ? number + 12 : number + 2;
		const Data::RosterEntry &entry = _G(savegame).entry(number);

		Common::String line = Common::String::format("%02d", number);
		if (!entry.isEmpty()) {
			line += entry.isInParty() ? '*' : '-';
			line += ' ';
			line += (char)entry._sex;
			line += (char)entry._race;
			line += (char)entry._class;
			line += (char)entry._status;
			line += ' ';
			line += entry._name;
		}

		s.writeString(Common::Point(col, row), line);
	}

	drawSpacePrompt(s);
}

bool Register::msgKeypress(const KeypressMessage &msg) {
	if (isSpaceKey(msg))
		close();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
