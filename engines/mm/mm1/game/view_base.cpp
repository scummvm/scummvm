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

#include "mm/mm1/game/view_base.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

ViewBase::ViewBase(UIElement *owner) : Views::TextView("View", owner) {
}

void ViewBase::update() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;

	_mapOffset = maps._mapPos.y * MAP_W + maps._mapPos.x;
	maps._currentWalls = map._walls[_mapOffset];
	maps._currentState = map._states[_mapOffset];

	if (maps._currentState & Maps::CELL_DARK) {
		if (g_globals->_spells._s.light) {
			g_globals->_spells._s.light--;
		} else {
			goto darkness;
		}
	}

	if ((map[46] & 1) && !g_globals->_spells._s.light) {
darkness:
		// TODO
		return;
	}
}

bool ViewBase::msgAction(const ActionMessage &msg) {
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

bool ViewBase::msgGame(const GameMessage &msg) {
	/*
	if (msg._name == "DISPLAY") {
		replaceView(this);
		return true;
	}
	*/

	return TextView::msgGame(msg);
}

void ViewBase::turnLeft() {
	g_globals->_maps.turnLeft();
}

void ViewBase::turnRight() {
	g_globals->_maps.turnRight();
}

void ViewBase::forward() {
	g_globals->_maps.forward();
}

void ViewBase::backwards() {
	g_globals->_maps.backwards();
}

void ViewBase::obstructed() {
	// TODO
}

} // namespace Game
} // namespace MM1
} // namespace MM
