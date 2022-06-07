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

#include "common/util.h"
#include "mm/mm1/views/game_messages.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

GameMessages::GameMessages(UIElement *owner) :
		TextView("GameMessages", owner) {
	_bounds = getLineBounds(20, 24);
}

void GameMessages::draw() {
	clearSurface();
	for (uint i = 0; i < MIN(_lines.size(), 4U); ++i)
		writeString(0, i, _lines[i]);

	_lines.clear();
}

bool GameMessages::msgInfo(const InfoMessage &msg) {
	_lines = msg._lines;
	redraw();
	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
