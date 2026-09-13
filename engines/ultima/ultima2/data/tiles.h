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

#ifndef ULTIMA2_DATA_TILES_H
#define ULTIMA2_DATA_TILES_H

#include "graphics/surface.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

enum TileId {
	TILE_WATER = 0, TILE_SWAMP = 1, TILE_GRASS = 2, TILE_FOREST = 3, TILE_MOUNTAIN = 4,
	TILE_UNKNOWN_5 = 5, TILE_TOWN = 6, TILE_TOWER = 7, TILE_CASTLE = 8,
	TILE_DUNGEON_ENTRANCE = 9, TILE_SIGNPOST = 10, TILE_SEA_MONSTER = 11, TILE_ORC = 12,
	TILE_DAEMON = 13, TILE_DEVIL = 14, TILE_BALRON = 15, TILE_MINAX = 16, TILE_HORSE = 17,
	TILE_SHIP = 18, TILE_AIRPLANE = 19, TILE_ROCKET = 20, TILE_SHIELD = 21, TILE_SWORD = 22,
	TILE_FORCEFIELD = 23, TILE_GUARD = 24, TILE_JESTER = 25, TILE_SHOPKEEP = 26,
	TILE_UNKNOWN_27 = 27, TILE_ROAD = 28, TILE_EMPTY = 29, TILE_WALL = 30,
	TILE_EMPTY_COUNTER = 31,
	TILE_A = 32, TILE_B = 33, TILE_C = 34, TILE_D = 35, TILE_E = 36, TILE_F = 37, TILE_G = 38,
	TILE_H = 39, TILE_I_DOOR = 40, TILE_J = 41, TILE_K = 42, TILE_L = 43, TILE_M = 44,
	TILE_N = 45, TILE_O = 46, TILE_P = 47, TILE_MOONGATE = 48, TILE_R = 49, TILE_S = 50,
	TILE_T = 51, TILE_U = 52, TILE_V = 53, TILE_W = 54, TILE_X = 55, TILE_Y = 56, TILE_Z = 57,
	TILE_COUNTER_END_RIGHT = 58, TILE_COUNTER_END_LEFT = 59,
	TILE_FIGHTER = 60, TILE_CLERIC = 61, TILE_MAGE = 62, TILE_THIEF = 63,
	TILE_COUNT = 64
};

constexpr int TILE_WIDTH = 16;
constexpr int TILE_HEIGHT = 16;

/**
 * Decodes the 64 overworld/town tile graphics embedded directly in
 * ULTIMAII.EXE into 16x16 CLUT8 surfaces, indexed into the same 4-color
 * CGA palette as the pic??? full-screen art (see Gfx::PicDecoder).
 */
void loadTiles(Graphics::Surface tiles[TILE_COUNT]);

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
