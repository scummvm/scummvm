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

#include "common/compression/deflate.h"
#include "common/config-manager.h"

#include "common/base64.h"

#include "common/memstream.h"
#include "common/bufferedstream.h"

#include "scumm/he/intern_he.h"

#include "scumm/he/moonbase/map_data.h"
#include "scumm/he/moonbase/map_main.h"

#include "scumm/he/moonbase/map_spiff.h"
#include "scumm/he/moonbase/map_katton.h"

namespace Scumm {

Map::Map(ScummEngine_v100he *vm) : _vm(vm), _rnd("moonbase") {
	_mapGenerated = false;
	_generatedMap = nullptr;

	_generator = 0;
	_size = 0;
	_seed = 0;
	_tileset = 0;
	_energy = -1;
	_terrain = -1;
	_water = -1;

	_encodedMap = Common::String();
}

Map::~Map() {
	deleteMap();
}

bool Map::generateNewMap() {
	deleteMap();

	if (!ConfMan.getBool("generate_random_maps"))
		return false;

	// Show a dialog allowing the user to customize options.
	if (!_vm->mapGeneratorDialog(!strcmp(_vm->_game.variant, "Demo")))
		// They have clicked cancel, abort.
		return false;

	// Create a new seed just for the below values.  This is to
	// ensure these are truly random after generating a previous
	// map (or to debug with a prefixed seed).
	_rnd.generateNewSeed();

	if (ConfMan.hasKey("map_size"))
		_size = ConfMan.getInt("map_size");
	if (_size < 4 || _size > 10)
		// Don't randomly pick nonstandard map sizes.
		_size = _rnd.getRandomNumberRngSigned(4, 8);

	_size *= 8;

	if (ConfMan.hasKey("map_algorithm"))
		_generator = ConfMan.getInt("map_algorithm");
	if (_generator < SPIFF_GEN || _generator > KATTON_GEN)
		_generator = _rnd.getRandomNumberRng(1, 2);

	if (ConfMan.hasKey("map_tileset"))
		_tileset = ConfMan.getInt("map_tileset");
	if (_tileset < 1 || _tileset > 6)
		_tileset = _rnd.getRandomNumberRngSigned(1, 6);

	if (!strcmp(_vm->_game.variant, "Demo") && (_tileset == 1 ||
		_tileset == 3 || _tileset == 5)) {
		// Demo version only has tilesets 1, 2, 4 and 6.
		switch (_rnd.getRandomNumber(3)) {
		case 0:
			_tileset = 1;
			break;
		case 1:
			_tileset = 2;
			break;
		case 2:
			_tileset = 4;
			break;
		default:
			_tileset = 6;
		}
	}

	if (ConfMan.hasKey("map_energy"))
		_energy = ConfMan.getInt("map_energy");
	if (_energy < 0 || _energy > 6)
		// Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
		_energy = _rnd.getRandomNumberRngSigned(2, 4);

	if (ConfMan.hasKey("map_terrain"))
		_terrain = ConfMan.getInt("map_terrain");
	if (_terrain < 0 || _terrain > 6)
		// Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
		_terrain = _rnd.getRandomNumberRngSigned(2, 4);

	if (ConfMan.hasKey("map_water"))
		_water = ConfMan.getInt("map_water");
	if (_water < 0 || _water > 6)
		// Only use [2, 3, 4] of the legal [0, 1, 2, 3, 4, 5, 6]
		_water = _rnd.getRandomNumberRngSigned(2, 4);

	// 32767 is RAND_MAX on Windows
	int seed = _rnd.getRandomNumber(32767);

	debug(1, "Map: Generating new map with info: generator = %d, seed = %d, size = %d, tileset = %d , energy = %d, terrain = %d, water = %d.", _generator, getSeed(), _size, _tileset, _energy, _terrain, _water);
	switch (_generator) {
	case SPIFF_GEN:
	{
		SpiffGenerator spiff = SpiffGenerator(seed);
		_generatedMap = spiff.generateMap(_water, _tileset, _size, _energy, _terrain);
		break;
	}
	case KATTON_GEN:
	{
		KattonGenerator katton = KattonGenerator(seed);
		_generatedMap = katton.generateMap(_water, _tileset, _size, _energy, _terrain);
		break;
	}
	default:
		error("Map: Got unknown generator: %d", _generator);
		return false;
	}

	// Encode the newly generated map file into base64 to transmit over the wire:
	_encodedMap = Common::b64EncodeData(_generatedMap, sizeof(MapFile));
	debug(2, "Map: Base64: %s", _encodedMap.c_str());

	_mapGenerated = true;
	return true;
}

bool Map::generateMapWithInfo(Common::String encodedMap, uint8 generator, int seed, int mapSize, int tileset, int energy, int terrain, int water) {
	deleteMap();

	_generator = generator;
	_seed = seed;
	_size = mapSize;
	_tileset = tileset;
	_energy = energy;
	_terrain = terrain;
	_water = water;
	_encodedMap = encodedMap;

	// Decode base64 encoded map file
	debug(2, "Map: Generating map with base64: encodedMap: %s, generator = %d, seed = %d, mapSize = %d, tileset = %d , energy = %d, terrain = %d, water = %d.", encodedMap.c_str(), generator, getSeed(), mapSize, tileset, energy, terrain, water);
	_generatedMap = new MapFile();
	bool success = Common::b64DecodeData(encodedMap, _generatedMap);
	if (!success) {
		warning("Map: Error has occurred while decoding map data from base64");
		return false;
	}
	_mapGenerated = true;

	return true;
}

void Map::deleteMap() {
	if (_mapGenerated) {
		// Delete old map.
		delete _generatedMap;
		_generatedMap = nullptr;

		_mapGenerated = false;
		_generator = 0;
		_size = 0;
		_seed = 0;
		_tileset = 0;
		_energy = -1;
		_terrain = -1;
		_water = -1;
		_encodedMap = "";
		debug(1, "Map: Deleted.");
	}
}

Common::SeekableReadStream *Map::makeWiz() {
	unsigned short wiz [139][139];
	int i, j;

	Common::MemoryReadStream *wizTemplate = new Common::MemoryReadStream(Template_wiz, ARRAYSIZE(Template_wiz));
	Common::SeekableReadStream *stream = Common::wrapCompressedReadStream(wizTemplate);

	stream->seek(0x0448);
	for (j = 0; j < 139; ++j) {
		for (i = 0; i < 139; ++i) {
			uint16 data = stream->readUint16LE();
			wiz[i][j] = data;
		}
	}
	delete stream;

	for (j = 0; j < _energy * 9; j++) {
		for (i = 30; i < 51; i++) {
			wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (_energy * 9)))) / 8 * 32 + ((int) (80 + j * 80 / (_energy * 9)) / 8 * 1024);
		}
	}

	for (j = 0; j < _terrain * 9; j++) {
		for (i = 61; i < 82; i++) {
			wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (_terrain*9))))/8 * 32 + ((int) (80 + j * 80 / (_terrain * 9)) / 8 * 1024);
		}
	}

	for (j = 0; j < _water * 9; j++) {
		for (i = 92; i < 113; i++) {
			wiz[i][91 - j] = ((255) / 8 ) + ((int) (130 - (j * 100 / (_water * 9)))) / 8 * 32 + ((int) (80 + j * 80 / (_water*9)) / 8 * 1024);
		}
	}

	// Re-read the template (to avoid compressed stream seeking):
	wizTemplate = new Common::MemoryReadStream(Template_wiz, ARRAYSIZE(Template_wiz));
	stream = Common::wrapCompressedReadStream(wizTemplate);
	byte *pwiz = (byte *)malloc(TEMPLATE_WIZ_SIZE);
	stream->read(pwiz, TEMPLATE_WIZ_SIZE);
	delete stream;

	Common::SeekableMemoryWriteStream ws = Common::SeekableMemoryWriteStream(pwiz, TEMPLATE_WIZ_SIZE);
	ws.seek(0x0448);
	for (j = 0; j < 139; j++) {
		for (i = 0; i < 139; i++) {
			ws.writeUint16LE(wiz[i][j]);
		}
	}

	return new Common::MemoryReadStream(pwiz, TEMPLATE_WIZ_SIZE, DisposeAfterUse::YES);
}

Common::SeekableReadStream *Map::substituteFile(const byte *fileName) {
	if (_mapGenerated) {
		// Worth noting here that the game opens these files more than once.
		// The exact scenario for the .thm and .wiz files is that the game
		// opens it first to make sure that it exists and readable, closes it,
		// and calls processWizImage to open the file again to actually read
		// the data inside.
		if (!strcmp((const char *)fileName, "map\\moon001.thm")) {
			// Load compressed thumbnail data from header.
			Common::MemoryReadStream *templateThm = new Common::MemoryReadStream(Template_thm, ARRAYSIZE(Template_thm));
			Common::SeekableReadStream *stream = Common::wrapCompressedReadStream(templateThm);

			// Read the uncompressed data into memory
			// (This is done to avoid compressed stream seeking)
			byte *thumbnail = (byte *)malloc(TEMPLATE_THM_SIZE);
			stream->read(thumbnail, TEMPLATE_THM_SIZE);
			delete stream;

			// And return a new ReadStream for it.
			return new Common::MemoryReadStream(thumbnail, TEMPLATE_THM_SIZE, DisposeAfterUse::YES);
		}

		if (!strcmp((const char *)fileName, "map\\moon001.wiz")) {
			return makeWiz();
		}

		if (!strcmp((const char *)fileName, "map\\moon001.map") ||
		    !strcmp((const char *)fileName, "user\\Temp.map")) {
			// (The Temp.map name is used when the game saves the map alongside
			// replay data)
			// Return new ReadStream but do not dispose it.  We'll handle
			// that ourselves.
			return new Common::MemoryReadStream((byte *)_generatedMap, sizeof(MapFile));
		}
	}
	return nullptr;
}

} // End of namespace Scumm
