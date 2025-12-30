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

#include "ultima/ultima0/views/world_map.h"
#include "ultima/ultima0/ultima0.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/gfx/map.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

WorldMap::WorldMap() : View("WorldMap") {
}

bool WorldMap::msgFocus(const FocusMessage &msg) {
	return true;
}

void WorldMap::draw() {
	auto s = getSurface();

	// Draw the map
	Graphics::ManagedSurface mapArea(s, Common::Rect(0, 0, s.w, s.h - Gfx::CHAR_HEIGHT * 4));
	Gfx::Map::draw(&mapArea);

	// Allow the status area to draw
	View::draw();
}

bool WorldMap::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_INFO:
		// Show character info screen
		replaceView("Info");
		break;
	}

	return true;
}

bool WorldMap::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_q:
		// "Quit" in the original which merely saves the game. For ScummVM,
		// we open the GMM, allowing the user to either save or quit
		g_engine->openMainMenuDialog();
		break;

	default:
		break;
	}

	return true;
}

void WorldMap::endOfTurn() {
	auto &player = g_engine->_player;

	if (player.Attr[AT_HP] <= 0)
		replaceView("Dead");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
