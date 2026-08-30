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

#ifndef ULTIMA1_DATA_TILES_H
#define ULTIMA1_DATA_TILES_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

constexpr int OVERWORLD_TILES_COUNT = 52;
constexpr int CITY_TILES_COUNT = 51;

enum OverworldTileId {
	TILE_OCEAN = 0,
	TILE_GRASS = 1,
	TILE_WOODS = 2,
	TILE_MOUNTAINS = 3,
	TILE_CASTLE1 = 4,
	TILE_CASTLE2 = 5,
	TILE_SIGNPOST = 6,
	TILE_CITY1 = 7,
	TILE_CITY2 = 8,
	TILE_PLAYER = 10,
	TILE_HORSE = 11,
	TILE_CART = 12,
	TILE_RAFT = 13,
	TILE_FRIGATE1 = 14,
	TILE_FRIGATE2 = 15,
	TILE_AIRCAR = 16,
	TILE_SHUTTLE = 17,
	TILE_TIME_MACHINE = 18,
	TILE_FIRST_MONSTER = 19,
	TILE_WANDERING_WARLOCK = 47,
	TILE_LAST_MONSTER = 47
};

enum LocationTileId {
	LOCTILE_WALL = 0,
	LOCTILE_PLAYER = 18
};

/**
 * Loads the overworld tile sheet (52 16x16 tiles, 4-bit EGA planar,
 * row-interleaved: each 16-pixel row is 4 planes x 2 bytes) into a
 * caller-supplied array of TILE_COUNT surfaces
 * @param filename		Filename to load
 * @param tiles			Tiles array to load into
 * @param count			Number of tiles
 * @param tileSize		Width/Height of individual tiles
 */
extern void loadTiles(const char *filename, Graphics::ManagedSurface *tiles, int count, int size);

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
