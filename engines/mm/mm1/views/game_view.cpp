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

#include "mm/mm1/views/game_view.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

GameView::GameView() : TextView("View") {
}

void GameView::draw() {

}

bool GameView::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_FORWARDS:
		forward();
		break;
	case KEYBIND_BACKWARDS:
		backwards();
		break;
	case KEYBIND_TURN_LEFT:
		turnLeft();
		break;
	case KEYBIND_TURN_RIGHT:
		turnRight();
		break;
	default:
		return TextView::msgAction(msg);
	}

	return true;
}

bool GameView::msgGame(const GameMessage &msg) {
	/*
	if (msg._name == "DISPLAY") {
		replaceView(this);
		return true;
	}
	*/

	return TextView::msgGame(msg);
}

void GameView::turnLeft() {
	g_globals->_maps.turnLeft();
}

void GameView::turnRight() {
	g_globals->_maps.turnRight();
}

void GameView::forward() {
	g_globals->_maps.forward();
}

void GameView::backwards() {
	g_globals->_maps.backwards();
}

} // namespace Views
} // namespace MM1
} // namespace MM
