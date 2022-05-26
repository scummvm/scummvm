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

#include "mm/mm1/views/game.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

Game::Game() : TextView("Game"),
		_commands(this), _messages(this), _party(this) {
}

void Game::draw() {
	// Clear the screen, and then call superclass
	// to render the subviews within the view
	clearScreen();
	UIElement::draw();
}

bool Game::msgKeypress(const KeypressMessage &msg) {
	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
