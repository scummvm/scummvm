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

#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "mm/mm1/events.h"
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/maps/map00.h"

namespace MM {
namespace MM1 {

Maps::Maps *g_maps;

namespace Maps {

/** PLACEHOLDER MAPS **/
#define PLACEHOLDER(SUFFIX, INDEX, NAME, ID) \
	class Map##SUFFIX : public Map { \
	public: \
		Map##SUFFIX() : Map(INDEX, NAME, ID) {} \
		void special() override {} \
		void load() override { \
			Map::load(); \
			warning("TODO: Map not implemented yet"); \
		} \
	}

PLACEHOLDER(01, 1, "portsmit", 0xC03);
PLACEHOLDER(02, 2, "algary", 0x203);
PLACEHOLDER(03, 3, "dusk", 0x802);
PLACEHOLDER(04, 4, "erliquin", 0x0B1A);
PLACEHOLDER(05, 5, "cave1", 0x0A11);
PLACEHOLDER(06, 6, "cave2", 0x1);
PLACEHOLDER(07, 7, "cave3", 0xC01);
PLACEHOLDER(08, 8, "cave4", 0x202);
PLACEHOLDER(09, 9, "cave5", 0x5);
PLACEHOLDER(10, 10, "cave6", 0x51B);
PLACEHOLDER(11, 11, "cave7", 0x212);
PLACEHOLDER(12, 12, "cave8", 0x601);
PLACEHOLDER(13, 13, "cave9", 0xA00);
PLACEHOLDER(14, 14, "areaa1", 0xF01);
PLACEHOLDER(15, 15, "areaa2", 0x502);
PLACEHOLDER(16, 16, "areaa3", 0xB02);
PLACEHOLDER(17, 17, "areaa4", 0x103);
PLACEHOLDER(18, 18, "areab1", 0xA00);
PLACEHOLDER(19, 19, "areab2", 0x703);
PLACEHOLDER(20, 20, "areab3", 0x101);
PLACEHOLDER(21, 21, "areab4", 0xD03);
PLACEHOLDER(22, 22, "areac1", 0x304);
PLACEHOLDER(23, 23, "areac2", 0xA11);
PLACEHOLDER(24, 24, "areac3", 0x904);
PLACEHOLDER(25, 25, "areac4", 0xF04);
PLACEHOLDER(26, 26, "aread1", 0x505);
PLACEHOLDER(27, 27, "aread2", 0xB05);
PLACEHOLDER(28, 28, "aread3", 0x106);
PLACEHOLDER(29, 29, "aread4", 0x801);
PLACEHOLDER(30, 30, "areae1", 0x112);
PLACEHOLDER(31, 31, "areae2", 0x706);
PLACEHOLDER(32, 32, "areae3", 0xB1A);
PLACEHOLDER(33, 33, "areae4", 0x11B);
PLACEHOLDER(34, 34, "doom", 0x706);
PLACEHOLDER(35, 35, "blackrn", 0xF08);
PLACEHOLDER(36, 36, "blackrs", 0x508);
PLACEHOLDER(37, 37, "qvl1", 0xF03);
PLACEHOLDER(38, 38, "qvl2", 0x703);
PLACEHOLDER(39, 39, "rwl1", 0xF02);
PLACEHOLDER(40, 40, "rwl2", 0x702);
PLACEHOLDER(41, 41, "enf1", 0xF04);
PLACEHOLDER(42, 42, "enf2", 0x704);
PLACEHOLDER(43, 43, "whitew", 0xA11);
PLACEHOLDER(44, 44, "dragad", 0x107);
PLACEHOLDER(45, 45, "udrag1", 0xF05);
PLACEHOLDER(46, 46, "udrag2", 0xA00);
PLACEHOLDER(47, 47, "udrag3", 0x705);
PLACEHOLDER(48, 48, "demon", 0x412);
PLACEHOLDER(49, 49, "alamar", 0xB07);
PLACEHOLDER(50, 50, "pp1", 0xF01);
PLACEHOLDER(51, 51, "pp2", 0x701);
PLACEHOLDER(52, 52, "pp3", 0xE00);
PLACEHOLDER(53, 53, "pp4", 0x201);
PLACEHOLDER(54, 54, "astral", 0xB1A);
#undef PLACEHOLDER

static const byte LOOKUPS_START[4] = { 0, 0, 14, 34 };
static const byte COLOR_OFFSET[55] = {
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1
};

static const uint16 TILE_AREA1[] = {
	0x10D, 0x0B0B, 0x50A, 0x11A, 0x0B18, 0x517
};
static const uint16 TILE_AREA2[] = {
	0xB0B, 0x50A, 0x10D, 0x0F08, 0x907, 0x11A, 0x0B18, 0x517
};
static const uint16 TILE_AREA3[] = {
	0xB0B, 0x10D, 0x517, 0x0B18, 0x11A, 0x50A
};

static const uint16 *TILE_AREAS[3] = { TILE_AREA1, TILE_AREA2, TILE_AREA3 };
static const byte TILE_OFFSET[3] = { 1,  7, 15 };

#define RESOURCE_TILES_COUNT 12
static const uint16 TILE_WIDTHS[RESOURCE_TILES_COUNT] = {
	32, 40, 24, 16, 32, 40, 24, 16, 176, 96, 48, 16
};
static const uint16 TILE_HEIGHTS[RESOURCE_TILES_COUNT] = {
	128, 96, 64, 32, 128, 96, 64, 32, 96, 64, 32, 16
};


Maps::Maps() {
	g_maps = this;
	Common::fill(&_data1[0], &_data1[32], 0);

	_maps.push_back(new Map00());
	_maps.push_back(new Map01());
	_maps.push_back(new Map02());
	_maps.push_back(new Map03());
	_maps.push_back(new Map04());
	_maps.push_back(new Map05());
	_maps.push_back(new Map06());
	_maps.push_back(new Map07());
	_maps.push_back(new Map08());
	_maps.push_back(new Map09());
	_maps.push_back(new Map10());
	_maps.push_back(new Map11());
	_maps.push_back(new Map12());
	_maps.push_back(new Map13());
	_maps.push_back(new Map14());
	_maps.push_back(new Map15());
	_maps.push_back(new Map16());
	_maps.push_back(new Map17());
	_maps.push_back(new Map18());
	_maps.push_back(new Map19());
	_maps.push_back(new Map20());
	_maps.push_back(new Map21());
	_maps.push_back(new Map22());
	_maps.push_back(new Map23());
	_maps.push_back(new Map24());
	_maps.push_back(new Map25());
	_maps.push_back(new Map26());
	_maps.push_back(new Map27());
	_maps.push_back(new Map28());
	_maps.push_back(new Map29());
	_maps.push_back(new Map30());
	_maps.push_back(new Map31());
	_maps.push_back(new Map32());
	_maps.push_back(new Map33());
	_maps.push_back(new Map34());
	_maps.push_back(new Map35());
	_maps.push_back(new Map36());
	_maps.push_back(new Map37());
	_maps.push_back(new Map38());
	_maps.push_back(new Map39());
	_maps.push_back(new Map40());
	_maps.push_back(new Map41());
	_maps.push_back(new Map42());
	_maps.push_back(new Map43());
	_maps.push_back(new Map44());
	_maps.push_back(new Map45());
	_maps.push_back(new Map46());
	_maps.push_back(new Map47());
	_maps.push_back(new Map48());
	_maps.push_back(new Map49());
	_maps.push_back(new Map50());
	_maps.push_back(new Map51());
	_maps.push_back(new Map52());
	_maps.push_back(new Map53());
	_maps.push_back(new Map54());
}

Maps::~Maps() {
	for (uint i = 0; i < _maps.size(); ++i)
		delete _maps[i];
	g_maps = nullptr;
}

void Maps::load(uint mapId) {
	_mapId = mapId;
	_currentMap = _maps[mapId];
	_currentMap->load();
	Common::fill(&_data1[0], &_data1[32], 0);
}

void Maps::select(uint16 id, byte section) {
	uint mapId = getIndex(id, section);
	load(mapId);
}

void Maps::display(uint16 id, byte section) {
	select(id, section);
	loadTiles();
	g_events->send("Game", GameMessage("DISPLAY"));
}

void Maps::loadTown(TownId townId) {
	switch (townId) {
	case SORPIGAL:
		town15setup();
		_mapPos = Common::Point(8, 3);
		display(0x604);
		break;

	case PORTSMITH:
		town23setup();
		_mapPos = Common::Point(3, 12);
		display(0xc03);
		break;

	case ALGARY:
		town23setup();
		_mapPos = Common::Point(14, 8);
		display(0x302);
		break;

	case DUSK:
		town4setup();
		_mapPos = Common::Point(11, 8);
		display(0x802);
		break;

	case ERLIQUIN:
		town15setup();
		_mapPos = Common::Point(4, 4);
		display(0xB1A);
		break;
	}

	_currentMap->_visited[_mapPos.y * MAP_W + _mapPos.x] = true;
}

void Maps::town15setup() {
	_forwardMask = DIRMASK_N;
	_leftMask = DIRMASK_W;
	_rightMask = DIRMASK_E;
	_backwardsMask = DIRMASK_S;

	_forwardOffset = MAP_W;
	_leftOffset = -1;
	_rightOffset = 1;
	_backwardsOffset = -MAP_W;
}

void Maps::town23setup() {
	_forwardMask = DIRMASK_W;
	_leftMask = DIRMASK_S;
	_rightMask = DIRMASK_N;
	_backwardsMask = DIRMASK_E;

	_forwardOffset = -1;
	_leftOffset = -MAP_W;
	_rightOffset = MAP_W;
	_backwardsOffset = 1;
}

void Maps::town4setup() {
	_forwardMask = DIRMASK_E;
	_leftMask = DIRMASK_N;
	_rightMask = DIRMASK_S;
	_backwardsMask = DIRMASK_W;

	_forwardOffset = 1;
	_leftOffset = MAP_W;
	_rightOffset = -MAP_W;
	_backwardsOffset = -1;
}

uint Maps::getIndex(uint16 id, byte section) {
	uint idx = LOOKUPS_START[section];

	// Find map by Id
	for (; idx < _maps.size() && id != _maps[idx]->getId(); ++idx) {}
	assert(idx < _maps.size());

	_colorOffset = COLOR_OFFSET[idx];
	return idx;
}

void Maps::loadTiles() {
	_loadArea = _currentMap->dataByte(1);
	_loadId = _currentMap->dataWord(2);
	_loadSection = 1;
	loadTile();

	_loadArea = _currentMap->dataByte(1);
	_loadId = _currentMap->dataWord(4);
	_loadSection = 2;
	loadTile();

	_loadArea = _currentMap->dataByte(1);
	_loadId = _currentMap->dataWord(6);
	_loadSection = 3;
	loadTile();
}

void Maps::loadTile() {
	assert(_loadArea >= 1 && _loadArea <= 3);
	const uint16 *arr = TILE_AREAS[_loadArea - 1];
	int ctr = TILE_OFFSET[_loadArea - 1];
	int entryIndex;

	for (; *arr != _loadId; ++arr, ++ctr) {
	}

	_loadFlag = 0xff;
	if (ctr >= 19) {
		if (ctr != 19)
			_loadFlag = 0xaa;
		ctr = 1;
	}

	// Get the entry from the wallpix.dta file
	entryIndex = ctr - 1;
	Gfx::DTA dta(WALLPIX_DTA);
	Common::SeekableReadStream *entry = dta.load(entryIndex);
	entry->skip(2);

	// Decode the tiles
	Common::Array<Graphics::ManagedSurface> &tiles =
		_tiles[_loadSection - 1];
	tiles.clear();
	tiles.resize(RESOURCE_TILES_COUNT);

	Gfx::ScreenDecoder decoder;
	for (int i = 0; i < RESOURCE_TILES_COUNT; ++i) {
		if (!decoder.loadStream(*entry,
			TILE_WIDTHS[i], TILE_HEIGHTS[i]))
			error("Failed decoding tile");

		tiles[i].copyFrom(decoder.getSurface());
	}
}

void Maps::turnLeft() {
	DirMask tempMask = _rightMask;
	_rightMask = _forwardMask;
	_forwardMask = _leftMask;
	_leftMask = _backwardsMask;
	_backwardsMask = tempMask;

	int8 tempOffset = _rightOffset;
	_rightOffset = _forwardOffset;
	_forwardOffset = _leftOffset;
	_leftOffset = _backwardsOffset;
	_backwardsOffset = tempOffset;
}

void Maps::turnRight() {
	DirMask tempMask = _leftMask;
	_leftMask = _forwardMask;
	_forwardMask = _rightMask;
	_rightMask = _backwardsMask;
	_backwardsMask = tempMask;

	int8 tempOffset = _leftOffset;
	_leftOffset = _forwardOffset;
	_forwardOffset = _rightOffset;
	_rightOffset = _backwardsOffset;
	_backwardsOffset = tempOffset;
}

void Maps::turnAround() {
	SWAP(g_maps->_forwardMask, g_maps->_backwardsMask);
	SWAP(g_maps->_leftMask, g_maps->_rightMask);
	SWAP(g_maps->_forwardOffset, g_maps->_backwardsOffset);
	SWAP(g_maps->_leftOffset, g_maps->_rightOffset);
}

void Maps::step(const Common::Point &delta) {
	_mapPos += delta;
	_currentMap->_visited[_mapPos.y * MAP_W + _mapPos.x] = true;

	int section = 0, id = 0;
	if (_mapPos.x < 0) {
		_mapPos.x = MAP_W - 1;
		id = _currentMap->dataWord(16);
		section = _currentMap->dataByte(14);
	} else if (_mapPos.x >= MAP_W) {
		_mapPos.x = 0;
		id = _currentMap->dataWord(11);
		section = _currentMap->dataByte(13);
	} else if (_mapPos.y < 0) {
		_mapPos.y = MAP_H - 1;
		id = _currentMap->dataWord(17);
		section = _currentMap->dataByte(19);
	} else if (_mapPos.y >= MAP_H) {
		_mapPos.y = 0;
		id = _currentMap->dataWord(8);
		section = _currentMap->dataByte(10);
	} else {
		return;
	}

	changeMap(id, section);
}

void Maps::changeMap(uint16 id, byte section) {
	// At this point, a new map is being entered
	select(id, section);
	loadTiles();

	g_events->send("Game", GameMessage("UPDATE"));
}

void Maps::clearSpecial() {
	_currentState &= ~CELL_SPECIAL;
	_currentMap->_states[_mapOffset] &= ~CELL_SPECIAL;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
