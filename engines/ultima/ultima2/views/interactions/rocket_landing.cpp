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

#include "ultima/ultima2/views/interactions/rocket_landing.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

constexpr uint32 STEP_DELAY = 100;

RocketLanding::RocketLanding() : Interaction("RocketLanding") {
}

bool RocketLanding::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	_lastStep = g_system->getMillis();

	writeString("(PRESS ANY KEY)");
	return Interaction::msgFocus(msg);
}

bool RocketLanding::tick() {
	uint32 now = g_system->getMillis();
	if (now - _lastStep >= STEP_DELAY) {
		_lastStep = now;

		// Sweeps steadily south, drifting a little east as it goes
		Data::Savegame &sg = _G(savegame);
		sg._mapY = (sg._mapY + 2) % Data::MAP_HEIGHT;
		if ((sg._mapY & 7) == 0)
			sg._mapX = (sg._mapX + 1) % Data::MAP_WIDTH;

		UIElement *map = g_engine->findView("OverworldMap");
		map->redraw();
		map->draw();
	}

	return Interaction::tick();
}

bool RocketLanding::msgKeypress(const KeypressMessage &msg) {
	Data::Savegame &sg = _G(savegame);
	close();
	writeString("\n");

	if (_G(map).tileAt(sg._mapX, sg._mapY) != Data::TILE_GRASS) {
		_G(logic)->playerDied();
		return true;
	}

	// Anywhere but Earth is where the rocket will next take off from
	if (sg._orbitTarget == 0) {
		sg._inSpace = false;
	} else {
		sg._launchMapX = sg._mapX;
		sg._launchMapY = sg._mapY;
		sg._inSpace = true;
	}

	_G(logic)->resumeTurn();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima
