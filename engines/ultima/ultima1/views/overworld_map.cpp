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

#include "ultima/ultima1/views/overworld_map.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;

bool OverworldMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);
	delayFrames(1);
	return Dialog::msgFocus(msg);
}

bool OverworldMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Dialog::msgUnfocus(msg);
}

void OverworldMap::timeout() {
	animateWater();
	++_animIndex;
	redraw();

	// Wait briefly before the next animation tick
	delayFrames(1);
}

void OverworldMap::animateWater() {
	const Graphics::ManagedSurface *tiles = _G(map).tiles();
	const Graphics::ManagedSurface &tile = tiles[Data::TILE_OCEAN];
	assert(tile.w == TILE_WIDTH && tile.h == TILE_HEIGHT);

	byte lastRow[TILE_WIDTH];
	byte *pixels = (byte *)tile.getPixels();

	// The bottom row wraps around to become the new top row, with
	// every other row shifting down by one to make way for it
	memcpy(lastRow, pixels + (TILE_HEIGHT - 1) * TILE_WIDTH, TILE_WIDTH);
	memmove(pixels + TILE_WIDTH, pixels, (TILE_HEIGHT - 1) * TILE_WIDTH);
	memcpy(pixels, lastRow, TILE_WIDTH);
}

int OverworldMap::animatedTileId(byte tileId) const {
	if (tileId == Data::TILE_CASTLE1) {
		return Data::TILE_CASTLE1 + ((_animIndex % 6) < 3 ? 1 : 0);
	}

	if (tileId == Data::TILE_CITY1) {
		// Redrawn biased 3:1 toward the waving-flag frame
		return Data::TILE_CITY1 + (((_animIndex % 4) == 0) ? 0 : 1);
	}

	return tileId;
}

void OverworldMap::draw() {
	Dialog::draw();

	auto &map = g_engine->_map;
	const Graphics::ManagedSurface *tiles = map.tiles();
	auto s = getSurface();

	prepareMapForDrawing();

	// Draw the visible map contents
	for (int oy = 0; oy < Data::OVERWORLD_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < Data::OVERWORLD_VISIBLE_WIDTH; ox++) {
			int tileId = animatedTileId(_mapTilesId[oy][ox]);
			if (tileId != -1) {
				const Graphics::ManagedSurface &tileImg = tiles[tileId];
				s.blitFrom(tileImg, Common::Point(ox * TILE_WIDTH + 8, oy * TILE_HEIGHT + 8));
			}
		}
	}
}

void OverworldMap::prepareMapForDrawing() {
	auto &map = g_engine->_map;
	int mapLeft = _G(savegame)._overworldPos.x - Data::OVERWORLD_VISIBLE_CENTER_X;
	int mapTop = _G(savegame)._overworldPos.y - Data::OVERWORLD_VISIBLE_CENTER_Y;

	for (int oy = 0; oy < Data::OVERWORLD_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < Data::OVERWORLD_VISIBLE_WIDTH; ox++) {
			int x = mapLeft + ox;
			int y = mapTop + oy;

			_mapTilesId[oy][ox] = _G(map).getTileAt(x, y);
		}
	}

	// Player is at center of the viewport. Save tiles around the player
	map._tilePlayerCenter = _mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y][Data::OVERWORLD_VISIBLE_CENTER_X];
	map._tilePlayerUp = _mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y - 1][Data::OVERWORLD_VISIBLE_CENTER_X];
	map._tilePlayerDown = _mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y + 1][Data::OVERWORLD_VISIBLE_CENTER_X];
	map._tilePlayerLeft = _mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y][Data::OVERWORLD_VISIBLE_CENTER_X - 1];
	map._tilePlayerRight = _mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y][Data::OVERWORLD_VISIBLE_CENTER_X + 1];

	// Place the player tile at the center
	_mapTilesId[Data::OVERWORLD_VISIBLE_CENTER_Y][Data::OVERWORLD_VISIBLE_CENTER_X] = map._playerTileId;
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
