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

#include "ultima/ultima1/views/location_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int TILE_WIDTH = 8;
constexpr int TILE_HEIGHT = 8;
constexpr int MAP_VISIBLE_WIDTH = 38;
constexpr int MAP_VISIBLE_HEIGHT = 18;

bool LocationMap::msgFocus(const FocusMessage &msg) {
	delayFrames(1);
	return UIElement::msgFocus(msg);
}

bool LocationMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Dialog::msgUnfocus(msg);
}

void LocationMap::timeout() {
	redraw();

	// Wait briefly before the next animation tick
	delayFrames(1);
}

void LocationMap::draw() {
	auto &map = g_engine->_map;
	const Graphics::ManagedSurface *tiles = map.tiles();
	auto s = getSurface();
	const auto &pos = _G(savegame)._locationPosition;

	Dialog::draw();

	// Draw the visible map contents
	for (int oy = 0; oy < MAP_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < MAP_VISIBLE_WIDTH; ox++) {
			int tileId = _G(map).getTileAt(ox, oy);
			if (tileId >= Data::CITY_TILES_COUNT)
				tileId = 1;

			const Graphics::ManagedSurface &tileImg = tiles[tileId];
			s.blitFrom(tileImg, Common::Point(ox * TILE_WIDTH + 8, oy * TILE_HEIGHT + 8));
		}
	}

	// Draw the player
	const Graphics::ManagedSurface &tileImg = tiles[Data::LOCTILE_PLAYER];
	s.blitFrom(tileImg, Common::Point(pos.x * TILE_WIDTH + 8, pos.y * TILE_HEIGHT + 8));
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
