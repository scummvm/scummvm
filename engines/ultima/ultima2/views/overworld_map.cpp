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

#include "ultima/ultima2/views/overworld_map.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

bool OverworldMap::msgFocus(const FocusMessage &msg) {
	g_engine->_map.clearTiles();
	delayFrames(1);
	return UIElement::msgFocus(msg);
}

void OverworldMap::timeout() {
	// Animate the water and forcefield tiles
	animateTile(g_engine->_tiles[0]);
	animateTile(g_engine->_tiles[23]);
	redraw();

	// Wait briefly before next tile animation
	delayFrames(1);
}

void OverworldMap::animateTile(Graphics::ManagedSurface &tile) {
	assert(tile.w == Data::TILE_WIDTH && tile.h == Data::TILE_HEIGHT);

	// Rotate 2 rows upward (top 2 rows move to bottom)
	const int ROWS_TO_ROTATE = 2;
	const int BYTES_TO_ROTATE = Data::TILE_WIDTH * ROWS_TO_ROTATE;

	// Temporary buffer to hold the rows being rotated
	byte tempBuffer[Data::TILE_WIDTH * ROWS_TO_ROTATE];

	byte *pixels = (byte *)tile.getPixels();

	// Save the first ROWS_TO_ROTATE rows to temp buffer
	memcpy(tempBuffer, pixels, BYTES_TO_ROTATE);

	// Shift all remaining rows up
	int remainingBytes = (tile.w * tile.h) - BYTES_TO_ROTATE;
	memmove(pixels, pixels + BYTES_TO_ROTATE, remainingBytes);

	// Copy saved rows to the end
	memcpy(pixels + remainingBytes, tempBuffer, BYTES_TO_ROTATE);
}

void OverworldMap::draw() {
	auto &map = g_engine->_map;
	auto s = getSurface();

	prepareMapForDrawing();

	// Draw the visible map contents
	for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++) {
		for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++) {
			uint8 tileId = map._mapTilesId[oy][ox];
			if (!(tileId & 0x80)) {
				const Graphics::ManagedSurface &tileImg = g_engine->_tiles[tileId / 2];
				s.blitFrom(tileImg, Common::Point(ox * Data::TILE_WIDTH, oy * Data::TILE_HEIGHT));
			}
		}
	}

	// Copy rendered tile IDs for next frame's dirty comparison
	for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++)
		for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++)
			map._priorTileIds[oy][ox] = map._mapTilesId[oy][ox] & 0x7F;
}

void OverworldMap::prepareMapForDrawing() {
	// Load the map if it's not already active
	auto &map = g_engine->_map;

	//int mapOffsetX = 0, mapOffsetY = 0;
	int mapLeft, mapTop;

	if (map._outsideMapTile != -1) {
		// Non-wrapping draw (used for indoor maps/dungeons with a border tile)
		mapLeft = (map._mapX - 9) + 1;
		mapTop = (map._mapY - 5) + 1;

		for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++) {
			for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++) {
				uint8 x = mapLeft + ox;
				uint8 y = mapTop + oy;

				uint8 tile;
				if (x >= 64 || y >= 64) {
					// Out of bounds: fill with the designated outside tile
					tile = map._outsideMapTile;
				} else {
					tile = (map._tiles[y][x] / 2) & 0xfe;
				}

				if (tile == map._priorTileIds[oy][ox]) {
					if (tile == 0) {
						if (map._flag1 == 0xFF)
							tile |= 0x80;
					} else {
						if (tile != 46)
							tile |= 0x80;
					}
				}

				map._mapTilesId[oy][ox] = tile;
			}
		}

	} else {
		// Normal (wrapping) draw - overworld map
		mapTop = (map._mapY - 5) & 63;
		mapLeft = (map._mapX - 9) & 63;

		for (int oy = 0; oy < Data::MAP_VISIBLE_HEIGHT; oy++) {
			for (int ox = 0; ox < Data::MAP_VISIBLE_WIDTH; ox++) {
				uint8 x = (mapLeft + ox) & 63;
				uint8 y = (mapTop + oy) & 63;
				uint8 tile = (map._tiles[y][x] / 2) & 0xfe;

				if (tile == map._priorTileIds[oy][ox]) {
					if (tile == 0) {
						if (map._flag1 == 0xFF)
							tile |= 0x80;
					} else {
						if (tile != 46)
							tile |= 0x80;
					}
				}

				map._mapTilesId[oy][ox] = tile;
			}
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

void OverworldMap::flashCircle(int deltaX, int deltaY) {
	Common::Point pt((Data::MAP_VISIBLE_CENTER_X + deltaX) * Data::TILE_WIDTH,
		(Data::MAP_VISIBLE_CENTER_Y + deltaY) * Data::TILE_HEIGHT);
	auto s = getSurface();

	// Show the circle
	s.xorBlitFrom(g_engine->_tiles[Data::kTileCircle], pt);
	g_engine->getScreen()->update();

	// Brief pause
	g_engine->pauseMillis();

	// Remove it
	s.xorBlitFrom(g_engine->_tiles[Data::kTileCircle], pt);
	g_engine->getScreen()->update();
}


} // namespace Views
} // namespace Ultima2
} // namespace Ultima
