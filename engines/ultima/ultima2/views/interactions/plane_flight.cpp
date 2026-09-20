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


#include "ultima/ultima2/views/interactions/plane_flight.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

constexpr uint32 STEP_DELAY = 100;

PlaneFlight::PlaneFlight() : Interaction("PlaneFlight") {
}

bool PlaneFlight::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_dx = 0;
	_dy = -1;
	_lastStep = g_system->getMillis();

	writeString("\n");
	prompt();
	return Interaction::msgFocus(msg);
}

void PlaneFlight::setHeading(int dx, int dy, const char *name) {
	_dx = dx;
	_dy = dy;
	writeString("%s\n", name);
	prompt();
}

void PlaneFlight::land() {
	Data::Savegame &sg = _G(savegame);

	if (_G(map).tileAt(sg._mapX, sg._mapY) == Data::TILE_GRASS) {
		writeString("LAND PLANE.\n");
		close();
		_G(logic)->resumeTurn();
	} else {
		writeString("LAND--NOT HERE!!!!!\n");
		prompt();
	}
}

bool PlaneFlight::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_UP: setHeading(0, -1, "NORTH"); break;
	case Common::KEYCODE_DOWN: setHeading(0, 1, "SOUTH"); break;
	case Common::KEYCODE_LEFT: setHeading(-1, 0, "WEST"); break;
	case Common::KEYCODE_RIGHT: setHeading(1, 0, "EAST"); break;
	default:
		if (toupper(msg.ascii) == 'L')
			land();
		break;
	}

	return true;
}

bool PlaneFlight::tick() {
	uint32 now = g_system->getMillis();
	if (now - _lastStep >= STEP_DELAY) {
		_lastStep = now;

		Data::Savegame &sg = _G(savegame);
		sg._mapX = (sg._mapX + _dx + Data::MAP_WIDTH) % Data::MAP_WIDTH;
		sg._mapY = (sg._mapY + _dy + Data::MAP_HEIGHT) % Data::MAP_HEIGHT;

		// The focused view is this dummy one, so the map has to be redrawn explicitly
		UIElement *map = g_engine->findView("OverworldMap");
		map->redraw();
		map->draw();
	}

	return Interaction::tick();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
