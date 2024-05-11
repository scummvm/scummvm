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

#ifndef SCUMM_HE_MOONBASE_MAP_MAIN_H
#define SCUMM_HE_MOONBASE_MAP_MAIN_H

#ifdef ENABLE_HE

#include "common/random.h"
#include "common/stream.h"

#include "engines/scumm/he/moonbase/map_mif.h"

#define SPIFF_GEN  1
#define KATTON_GEN 2

namespace Scumm {

class Map {
public:
	Map(ScummEngine_v100he *vm);
	~Map();

	bool generateNewMap();
	bool generateMapWithInfo(Common::String encodedMap, uint8 generator, int seed, int mapSize, int tileset, int energy, int terrain, int water);
	Common::SeekableReadStream *substituteFile(const byte *fileName);

	void deleteMap();

	uint8 getGenerator() const {
		return _generator;
	}

	int getSize() const {
		return _size;
	}

	int getSeed() const {
		return _seed;
	}

	int getTileset() const {
		return _tileset;
	}

	int getEnergy() const {
		return _energy;
	}

	int getTerrain() const {
		return _terrain;
	}

	int getWater() const {
		return _water;
	}

	bool mapGenerated() const {
		return _mapGenerated;
	}

	Common::String getEncodedMap() const {
		return _encodedMap;
	}

private:
	ScummEngine_v100he *_vm;

	// We require our own random number generator
	// so we can send and set seeds from online players to ensure
	// they're playing on the same generated map.
	Common::RandomSource _rnd;

	uint8 _generator;
	int _size;
	int _seed;
	int _tileset;
	int _energy;
	int _terrain;
	int _water;

	bool _mapGenerated;
	MapFile *_generatedMap;
	Common::String _encodedMap;

	Common::SeekableReadStream *makeWiz();
};

} // End of namespace Scumm


#endif // ENABLE_HE

#endif // SCUMM_HE_MOONBASE_MAP_MAIN_H
