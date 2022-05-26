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

#include "mm/mm1/views/quick_ref.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void QuickRef::draw() {
	clearScreen();
	writeString(STRING["dialogs.quick_ref.title"]);
	_textPos.x = 0;
	_textPos.y = 2;

	for (uint idx = 0; idx < PARTY_COUNT; ++idx) {
		
	}
}

bool QuickRef::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
