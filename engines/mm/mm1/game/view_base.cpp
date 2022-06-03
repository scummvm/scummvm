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
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Game {

ViewBase::ViewBase(UIElement *owner) : Views::TextView("View", owner) {
	Common::fill(&_arr2[0], &_arr2[8], 0);
}

void ViewBase::update() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;

	_mapOffset = maps._mapPos.y * MAP_W + maps._mapPos.x;
	maps._currentWalls = map._walls[_mapOffset];
	maps._currentState = map._states[_mapOffset];
	_isDark = false;

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
		_isDark = true;
	}

	// Redraw the screen
	redraw();
}

bool ViewBase::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_FORWARDS:
	case KEYBIND_STRAFE_LEFT:
	case KEYBIND_STRAFE_RIGHT:
		forward(msg._action);
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

bool ViewBase::msgFocus(const FocusMessage &msg) {
	update();
	return false;
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
	update();
}

void ViewBase::turnRight() {
	g_globals->_maps.turnRight();
	update();
}

void ViewBase::forward(KeybindingAction action) {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *g_globals->_maps._currentMap;

	//  Figure out direction mask
	byte mask = maps._forwardMask;
	if (action == KEYBIND_STRAFE_LEFT) {
		mask = maps._leftMask;
	} else if (action == KEYBIND_STRAFE_RIGHT) {
		mask = maps._rightMask;
	}

	// Get the delta X/Y from the direction
	Common::Point delta(0, 1);
	switch (mask) {
	case Maps::DIRMASK_E:
		delta = Common::Point(1, 0);
		break;
	case Maps::DIRMASK_W:
		delta = Common::Point(-1, 0);
		break;
	case Maps::DIRMASK_S:
		delta = Common::Point(0, -1);
		break;
	default:
		break;
	}

	// Check for obstructions
	if (maps._currentWalls & mask) {
		if (maps._currentState & 0x55 & mask) {
			barrier();
			return;
		}
	} else {
		if (maps._currentState & 0x55 & mask) {
			obstructed();
			return;
		}

		int offset;
		if (!(maps._currentWalls & mask & 0x55))
			offset = 1;
		else if (maps._currentWalls & mask & 0xaa)
			offset = 2;
		else
			offset = 0;

		if (map.dataByte(30 + offset) == 4 &&
				!g_globals->_spells._s.walk_on_water) {
			dialogVal(1);
			dialogMessage(STRING["movement.obstructed.cant_swim"]);
			return;
		}
	}

	Common::fill(&_arr2[0], &_arr2[8], 0);

	int maxVal = map.dataByte(29);
	if (g_engine->getRandomNumber(maxVal) == maxVal)
		_val1 = 1;

	g_globals->_maps.step(delta);
	update();
}

void ViewBase::backwards() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *g_globals->_maps._currentMap;

// Get the delta X/Y from the direction
	Common::Point delta(0, -1);
	switch (maps._backwardsMask) {
	case Maps::DIRMASK_E:
		delta = Common::Point(-1, 0);
		break;
	case Maps::DIRMASK_W:
		delta = Common::Point(1, 0);
		break;
	case Maps::DIRMASK_S:
		delta = Common::Point(0, 1);
		break;
	default:
		break;
	}

	if (maps._currentWalls & maps._backwardsMask) {
		dialogVal(1);
		checkPartyDead();
		return;
	}
	if (maps._currentState & 0x55 & maps._backwardsMask) {
		dialogVal(1);
		checkPartyDead();
		return;
	}

	Common::fill(&_arr2[0], &_arr2[8], 0);

	int maxVal = map.dataByte(29);
	if (g_engine->getRandomNumber(maxVal) == maxVal)
		_val1 = 1;

	g_globals->_maps.step(delta);
	update();
}

void ViewBase::obstructed() {
	// TODO
}

void ViewBase::barrier() {
	// TODO
}

void ViewBase::dialogVal(int num) {
	// TODO
}

void ViewBase::dialogMessage(const Common::String &msg) {
	// TODO
}

void ViewBase::checkPartyDead() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if ((c._condition & (ASLEEP | STONE | DEAD | BAD_CONDITION)) == 0)
			return;
	}

	// At this point, there's no good characters.
	// So redirect to the death screen
	replaceView("Dead");
}

} // namespace Game
} // namespace MM1
} // namespace MM
