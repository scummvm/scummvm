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

#include "common/system.h"
#include "ultima/ultima2/views/map.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

// The original animates while waiting for a command: the water tile scrolls
// four rows and the forcefield tile one row per step
constexpr uint32 ANIMATION_DELAY = 150;
constexpr int WATER_SCROLL_ROWS = 4;

// Without a command for this long, the original passes the turn for you
constexpr uint32 IDLE_DELAY = 5000;
constexpr int FORCEFIELD_SCROLL_ROWS = 1;

bool Map::msgFocus(const FocusMessage &msg) {
	_lastInput = g_system->getMillis();
	return View::msgFocus(msg);
}

bool Map::msgAction(const ActionMessage &msg) {
	_lastInput = g_system->getMillis();
	g_engine->_logic->action(msg._action);
	return true;
}

void Map::checkIdle() {
	uint32 now = g_system->getMillis();
	if (now - _lastInput >= IDLE_DELAY) {
		_lastInput = now;
		g_engine->_logic->action(KEYBIND_PASS);
	}
}

bool Map::tick() {
	uint32 now = g_system->getMillis();
	if (now - _lastAnimation >= ANIMATION_DELAY) {
		_lastAnimation = now;

		Graphics::Surface *tiles = tileGraphics();
		Data::scrollTileRows(tiles[Data::TILE_WATER], WATER_SCROLL_ROWS);
		Data::scrollTileRows(tiles[Data::TILE_FORCEFIELD], FORCEFIELD_SCROLL_ROWS);
		redraw();
	}

	checkIdle();
	return View::tick();
}

bool Map::msgKeypress(const KeypressMessage &msg) {
	_lastInput = g_system->getMillis();
	g_engine->_logic->keypress(msg);
	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
