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

#include "ultima/ultima1/views/game.h"
#include "ultima/ultima1/views/overworld_map.h"
#include "ultima/ultima1/views/location_map.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/shared/gfx/rect.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

// Colors used
constexpr int COLOR_BORDER = 1;  // blue - thick outer band
constexpr int COLOR_EDGE = 15;   // white - thin inner highlight line

Game::Game() : Dialog("Game"), _commands(this), _stats(this) {
	_commands.setBounds(TextRect(0, 20, 29, 24));
	_stats.setBounds(TextRect(31, 21, 39, 24));
}

Game::~Game() {
	delete _map;
}

bool Game::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_OVERWORLD);
	return View::msgFocus(msg);
}

bool Game::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return View::msgUnfocus(msg);
}

void Game::draw() {
	auto s = getSurface();
	s.clear();
	drawFrame();

	// Erase the frame's bottom section
	s.fillRect(Common::Rect(0, 153, 320, 200), 0);

	// Horizontal divider separating the map viewport (above) from the command/message log and stats panel
	s.fillRect(Common::Rect(0, 153, 320, 159), COLOR_BORDER);
	s.drawLine(7, 152, 312, 152, COLOR_EDGE);
	s.drawLine(0, 159, 240, 159, COLOR_EDGE);
	s.drawLine(247, 159, 319, 159, COLOR_EDGE);

	// Vertical divider between the command/message log (left) and the stats panel (right)
	s.fillRect(Common::Rect(241, 153, 247, 200), COLOR_BORDER);
	s.drawLine(240, 159, 240, 199, COLOR_EDGE);
	s.drawLine(247, 159, 247, 199, COLOR_EDGE);
}

bool Game::msgGame(const GameMessage &msg) {
	if (msg._name == "MAP") {
		if (_map) {
			// Remove current map view
			_children.remove(_map);
			delete _map;
			_map = nullptr;
		}

		if (msg._stringValue == "OVERWORLD")
			_map = new OverworldMap(this);
		else if (msg._stringValue == "CITY" || msg._stringValue == "CASTLE" || msg._stringValue == "PILLAR")
			_map = new LocationMap(this);
		else
			error("Invalid map type - %s", msg._stringValue.c_str());

		// Set the bounds
		_map->setBounds(TextRect(1, 1, 38, 19));
		return true;

	} else if (msg._name == "UPDATE") {
		if (msg._stringValue == "MAP")
			_map->redraw();
		if (msg._stringValue == "STATS")
			_stats.redraw();
		return true;
	}

	return Dialog::msgGame(msg);
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
