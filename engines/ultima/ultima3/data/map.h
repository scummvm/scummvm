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

#ifndef ULTIMA3_DATA_MAP_H
#define ULTIMA3_DATA_MAP_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr int MAP_SIZE = 64;

/**
 * A 64x64 location map, as used by the overworld, towns and castles. Each
 * cell holds a tile number multiplied by 4
 */
class Map {
private:
	byte _cells[MAP_SIZE * MAP_SIZE] = {};

public:
	/**
	 * Loads a map from the start of a .ULT file
	 */
	void load(const char *filename);

	/**
	 * Returns the raw cell value. The coordinates wrap around the map
	 */
	byte cell(int x, int y) const {
		return _cells[(y & (MAP_SIZE - 1)) * MAP_SIZE + (x & (MAP_SIZE - 1))];
	}

	void setCell(int x, int y, byte value) {
		_cells[(y & (MAP_SIZE - 1)) * MAP_SIZE + (x & (MAP_SIZE - 1))] = value;
	}

	/**
	 * Returns the number of the tile graphic in a cell
	 */
	byte tile(int x, int y) const {
		return cell(x, y) >> 2;
	}
};

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
