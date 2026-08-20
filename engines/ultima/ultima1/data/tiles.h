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

constexpr int TILE_COUNT = 52;
constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;

enum TileId {
	kTileOcean = 0,
	kTileGrass = 1,
	kTileWoods = 2,
	kTileCastle = 4,
	kTileCastleFlag = 5,
	kTileSignpost = 6,
	kTileCity = 7,
	kTileCityFlag = 8,
	kTileParty = 11,
	kTileHorse = 12,
	kTileCart = 13,
	kTileRaft = 14,
	kTileFrigate = 15,
	kTileAircar = 16,
	kTileShuttle = 17,
	kTileTimeMachine = 18,
	kTileFirstMonster = 19,
	kTileLastMonster = 47
};

/**
 * Loads the overworld tile sheet (52 16x16 tiles, 4-bit EGA planar,
 * row-interleaved: each 16-pixel row is 4 planes x 2 bytes) into a
 * caller-supplied array of TILE_COUNT surfaces
 */
void loadTiles(Graphics::ManagedSurface tiles[TILE_COUNT]);

} // namespace Data
} // namespace Ultima1
} // namespace Ultima

#endif
