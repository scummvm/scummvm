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

namespace Ultima {
namespace Ultima1 {
namespace Views {

bool LocationMap::msgFocus(const FocusMessage &msg) {
	_G(map).clearTiles();
	delayFrames(1);
	return UIElement::msgFocus(msg);
}

void LocationMap::timeout() {
	redraw();

	// Wait briefly before the next animation tick
	delayFrames(1);
}

int LocationMap::animatedTileId(byte tileId) const {
	if (tileId == Data::TILE_CASTLE1) {
		// Only redrawn on 2 of every 6 ticks; the other 4 ticks leave
		// whichever frame was last drawn untouched
		int offset = 2;
		if ((_animIndex % 6) == 0)
			offset = 0;
		else if ((_animIndex % 3) == 0)
			offset = 1;

		return (offset == 2) ? -1 : Data::TILE_CASTLE1 + offset;
	}

	if (tileId == Data::TILE_CITY1) {
		// Redrawn every tick, biased 3:1 toward the waving-flag frame
		return Data::TILE_CITY1 + (((_animIndex % 4) == 0) ? 0 : 1);
	}

	return tileId;
}

void LocationMap::draw() {
	auto &map = g_engine->_map;
	const Graphics::ManagedSurface *tiles = map.tiles();
	auto s = getSurface();

	prepareMapForDrawing();

	// Draw the visible map contents
	for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++) {
			int tileId = animatedTileId(map._mapTilesId[oy][ox]);
			if (tileId != -1) {
				const Graphics::ManagedSurface &tileImg = tiles[
					(_G(map)._currentMap != Data::MAP_OVERWORLD && tileId >= 50) ? 1 : tileId
				];
				s.blitFrom(tileImg, Common::Point(ox * Data::TILE_WIDTH, oy * Data::TILE_HEIGHT));
			}
		}
	}
}

void LocationMap::prepareMapForDrawing() {
	auto &map = g_engine->_map;

	// Get the map tiles. Unlike in the overworld, where the visible map is centered around the player,
	// location maps are always on-screen in their entirety
	for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++) {
			map._mapTilesId[oy][ox] = _G(map).getTileAt(ox, oy);
		}
	}

	// Player is at center of the viewport. Save tiles around the player
	map._tilePlayerCenter = map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y][Data::MAP_VISIBLE_CENTER_X];
	map._tilePlayerUp = map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y - 1][Data::MAP_VISIBLE_CENTER_X];
	map._tilePlayerDown = map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y + 1][Data::MAP_VISIBLE_CENTER_X];
	map._tilePlayerLeft = map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y][Data::MAP_VISIBLE_CENTER_X - 1];
	map._tilePlayerRight = map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y][Data::MAP_VISIBLE_CENTER_X + 1];

	// Place the player tile at the center
	map._mapTilesId[Data::MAP_VISIBLE_CENTER_Y][Data::MAP_VISIBLE_CENTER_X] = map._playerTileId;
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
