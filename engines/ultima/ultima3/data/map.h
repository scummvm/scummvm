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
#include "common/serializer.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr int MAP_SIZE = 64;
constexpr int CREATURE_COUNT = 32;

/**
 * The creatures wandering a location: the monsters of the overworld or the
 * people of a town or castle. Their tiles are also placed in the map cells
 * they stand on, with what's underneath them kept here
 */
struct Creatures {
	byte _tile[CREATURE_COUNT] = {};
	byte _floor[CREATURE_COUNT] = {};
	byte _x[CREATURE_COUNT] = {};
	byte _y[CREATURE_COUNT] = {};

	// The top two bits give how it behaves, and the rest what a person says
	byte _flags[CREATURE_COUNT] = {};
};

/**
 * A 64x64 location map, as used by the overworld, towns and castles. Each
 * cell holds a tile number multiplied by 4. The rest of a location's data,
 * which includes the things said there, follows it
 */
class Map {
private:
	static const int TEXT_SIZE = 0x180;
	static const int EXTRA_SIZE = 8;

	byte _cells[MAP_SIZE * MAP_SIZE] = {};

	// The offsets of the things said in a town, followed by the things
	byte _text[TEXT_SIZE] = {};

	// The whirlpool and moon phases on the overworld
	byte _extra[EXTRA_SIZE] = {};

	void synchronizeData(Common::Serializer &s);

public:
	Creatures _creatures;

	/**
	 * Returns true if a location data file exists
	 */
	static bool exists(const char *filename);

	/**
	 * Loads a location from a .ULT file
	 */
	void load(const char *filename);

	void synchronize(Common::Serializer &s);

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
	 * Returns one of the pieces of text kept with a town, which are what its
	 * people and signs say, or null if there isn't one
	 */
	const char *text(int index) const;

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
