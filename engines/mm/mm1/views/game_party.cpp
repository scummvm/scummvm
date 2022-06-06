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

#include "mm/mm1/views/game_party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

GameParty::GameParty(UIElement *owner) : TextView("GameParty", owner) {
	_bounds = getLineBounds(17, 19);
}

void GameParty::draw() {
	clearSurface();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		writeChar((i % 2) * 21, i / 2,
			c._condition ? '*' : ' ');
		writeString(Common::String::format("%d) %s",
			i + 1, c._name));
	}
}

bool GameParty::msgKeypress(const KeypressMessage &msg) {
	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
