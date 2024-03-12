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
#include "mm/mm1/game/view_base.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Game {

ViewBase::ViewBase(UIElement *owner) : Views::TextView("View", owner) {
}

void ViewBase::update() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;

	maps._mapOffset = maps._mapPos.y * MAP_W + maps._mapPos.x;
	maps._currentWalls = map._walls[maps._mapOffset];
	maps._currentState = map._states[maps._mapOffset];
	_isDark = false;

	if (maps._currentState & Maps::CELL_DARK) {
		if (g_globals->_activeSpells._s.light) {
			g_globals->_activeSpells._s.light--;
		} else {
			goto darkness;
		}
	}

	if ((map[Maps::MAP_FLAGS] & 1) && !g_globals->_activeSpells._s.light) {
darkness:
		_isDark = true;
	}

	// Refresh the view immediately, so things like the minimap will
	// update before special actions run
	g_events->redraw();
	g_events->drawElements();

	// Encounter checks
	g_globals->_encounters._encounterType = NORMAL_SURPRISED;
	if (maps._currentState & Maps::CELL_SPECIAL) {
		map.visitedSpecial();
		draw();

		map.special();
		return;

	} else if (_stepRandom) {
		g_globals->_encounters._encounterType = NORMAL_ENCOUNTER;
		_stepRandom = false;
		g_globals->_encounters.execute();
	} else {
		g_globals->_party.checkPartyDead();
	}
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
	return false;
}

bool ViewBase::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		replaceView("Game");
		return true;
	} else if (msg._name == "UPDATE") {
		replaceView("Game");
		update();
		return true;
	} else if (msg._name == "LOCATION") {
		_descriptionLine = STRING[Common::String::format(
			"dialogs.location.titles.%d", msg._value)];
		draw();
		return true;
	}

	return TextView::msgGame(msg);
}

bool ViewBase::msgHeader(const HeaderMessage &msg) {
	_descriptionLine = msg._name;
	draw();
	return true;
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
	Common::Point delta = g_maps->getMoveDelta(mask);

	// Check for obstructions
	if (!g_globals->_intangible) {
		if (maps._currentWalls & mask) {
			if (maps._currentState & 0x55 & mask) {
				obstructed(mask);
				redraw();
				return;
			}

			int offset;
			if (!(maps._currentWalls & mask & 0x55))
				offset = 1;
			else if (maps._currentWalls & mask & 0xaa)
				offset = 2;
			else
				offset = 0;

			if (map.dataByte(Maps::MAP_30 + offset) == 4 &&
				!g_globals->_activeSpells._s.walk_on_water) {
				Sound::sound(SOUND_1);
				_dialogMessage = STRING["movement.obstructed.cant_swim"];
				redraw();
				return;
			}
		} else {
			if (maps._currentState & 0x55 & mask) {
				barrier();
				redraw();
				return;
			}
		}
	}

	g_globals->_treasure.clear();

	int maxVal = map[Maps::MAP_29];
	if (g_engine->getRandomNumber(maxVal) == maxVal)
		_stepRandom = true;

	g_globals->_maps.step(delta);
	update();
}

void ViewBase::backwards() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *g_globals->_maps._currentMap;
	Common::Point delta = g_maps->getMoveDelta(maps._backwardsMask);

	if (!g_globals->_intangible) {
		if (maps._currentWalls & maps._backwardsMask) {
			Sound::sound(SOUND_1);
			g_globals->_party.checkPartyDead();
			return;
		}
		if (maps._currentState & 0x55 & maps._backwardsMask) {
			Sound::sound(SOUND_1);
			g_globals->_party.checkPartyDead();
			return;
		}
	}

	g_globals->_treasure.clear();

	int maxVal = map[Maps::MAP_29];
	if (g_engine->getRandomNumber(maxVal) == maxVal)
		_stepRandom = true;

	g_globals->_maps.step(delta);
	update();
}

void ViewBase::obstructed(byte mask) {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;
	Sound::sound(SOUND_1);

	int index = 32;
	if (!(maps._currentWalls & mask & 0x55))
		index = 31;
	else if (!(maps._currentWalls & mask & 0xaa))
		index = 30;

	_dialogMessage = STRING[Common::String::format(
		"movement.obstructed.%d", map.dataByte(index))];
}

void ViewBase::barrier() {
	_dialogMessage = STRING["movement.obstructed.barrier"];
	Sound::sound(SOUND_1);
}

} // namespace Game
} // namespace MM1
} // namespace MM
