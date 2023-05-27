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
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/maps/map00.h"
#include "mm/mm1/maps/map01.h"
#include "mm/mm1/maps/map02.h"
#include "mm/mm1/maps/map03.h"
#include "mm/mm1/maps/map04.h"
#include "mm/mm1/maps/map05.h"
#include "mm/mm1/maps/map06.h"
#include "mm/mm1/maps/map07.h"
#include "mm/mm1/maps/map08.h"
#include "mm/mm1/maps/map09.h"
#include "mm/mm1/maps/map10.h"
#include "mm/mm1/maps/map11.h"
#include "mm/mm1/maps/map12.h"
#include "mm/mm1/maps/map13.h"
#include "mm/mm1/maps/map14.h"
#include "mm/mm1/maps/map15.h"
#include "mm/mm1/maps/map16.h"
#include "mm/mm1/maps/map17.h"
#include "mm/mm1/maps/map18.h"
#include "mm/mm1/maps/map19.h"
#include "mm/mm1/maps/map20.h"
#include "mm/mm1/maps/map21.h"
#include "mm/mm1/maps/map22.h"
#include "mm/mm1/maps/map23.h"
#include "mm/mm1/maps/map24.h"
#include "mm/mm1/maps/map25.h"
#include "mm/mm1/maps/map26.h"
#include "mm/mm1/maps/map27.h"
#include "mm/mm1/maps/map28.h"
#include "mm/mm1/maps/map29.h"
#include "mm/mm1/maps/map30.h"
#include "mm/mm1/maps/map31.h"
#include "mm/mm1/maps/map32.h"
#include "mm/mm1/maps/map33.h"
#include "mm/mm1/maps/map34.h"
#include "mm/mm1/maps/map35.h"
#include "mm/mm1/maps/map36.h"
#include "mm/mm1/maps/map37.h"
#include "mm/mm1/maps/map38.h"
#include "mm/mm1/maps/map39.h"
#include "mm/mm1/maps/map40.h"
#include "mm/mm1/maps/map41.h"
#include "mm/mm1/maps/map42.h"
#include "mm/mm1/maps/map43.h"
#include "mm/mm1/maps/map44.h"
#include "mm/mm1/maps/map45.h"
#include "mm/mm1/maps/map46.h"
#include "mm/mm1/maps/map47.h"
#include "mm/mm1/maps/map48.h"
#include "mm/mm1/maps/map49.h"
#include "mm/mm1/maps/map50.h"
#include "mm/mm1/maps/map51.h"
#include "mm/mm1/maps/map52.h"
#include "mm/mm1/maps/map53.h"
#include "mm/mm1/maps/map54.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

Maps::Maps *g_maps;

namespace Maps {

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

static byte TILE_COLORS[18] = {
	0xe6, 0xe6, 0xe6, 0x72, 0x72, 0x72, 0x62, 0x62, 0x62,
	0x62, 0x62, 0xe1, 0x53, 0x53, 0xff, 0x43, 0x43, 0x63
};

Maps::Maps() {
	g_maps = this;

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
}

void Maps::synchronize(Common::Serializer &s) {
	// Store a count of the number of maps, just in case new ones
	// are added in later as easter eggs just for ScummVM
	int mapCount = _maps.size();
	s.syncAsByte(mapCount);

	for (int i = 0; i < mapCount; ++i) {
		s.syncBytes(_maps[i]->_visited, MAP_SIZE);
	}
}

void Maps::synchronizeCurrent(Common::Serializer &s) {
	// Save current map
	s.syncAsUint16LE(_id);
	s.syncAsByte(_section);

	s.syncAsByte(_mapPos.x);
	s.syncAsByte(_mapPos.y);
	s.syncAsByte(_forwardMask);

	if (s.isLoading()) {
		updateMasksOffsets();
		changeMap(_id, _section);
	}
}

void Maps::select(uint16 id, byte section) {
	_id = id;
	_section = section;

	uint mapId = getIndex(id, section);
	load(mapId);
}

void Maps::display(uint16 id, byte section) {
	select(id, section);
	loadTiles();
	g_events->send("Game", GameMessage("UPDATE"));
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
	default:
		break;
	}

	byte &visited = _currentMap->_visited[_mapPos.y * MAP_W + _mapPos.x];
	if (!visited)
		visited = VISITED_NORMAL;
}

void Maps::town15setup() {
	_forwardMask = DIRMASK_N;
	updateMasksOffsets();
}

void Maps::town23setup() {
	_forwardMask = DIRMASK_W;
	updateMasksOffsets();
}

void Maps::town4setup() {
	_forwardMask = DIRMASK_E;
	updateMasksOffsets();
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
	_loadArea = _currentMap->dataByte(MAP_1);
	_loadId = _currentMap->dataWord(MAP_2);
	_loadSection = 1;
	loadTile();

	_loadArea = _currentMap->dataByte(MAP_1);
	_loadId = _currentMap->dataWord(MAP_4);
	_loadSection = 2;
	loadTile();

	_loadArea = _currentMap->dataByte(MAP_1);
	_loadId = _currentMap->dataWord(MAP_6);
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
	byte colors = TILE_COLORS[entryIndex];
	assert(colors);
	decoder._indexes[0] = 0;
	decoder._indexes[1] = colors & 0xf;
	decoder._indexes[2] = (colors >> 4) & 0xf;
	decoder._indexes[3] = 15;

	for (int i = 0; i < RESOURCE_TILES_COUNT; ++i) {
		if (!decoder.loadStream(*entry,
			TILE_WIDTHS[i], TILE_HEIGHTS[i]))
			error("Failed decoding tile");

		tiles[i].copyFrom(decoder.getSurface());
	}
}

void Maps::turnLeft() {
	_forwardMask = _leftMask;
	updateMasksOffsets();
}

void Maps::turnRight() {
	_forwardMask = _rightMask;
	updateMasksOffsets();
}

void Maps::turnAround() {
	_forwardMask = _backwardsMask;
	updateMasksOffsets();
}

void Maps::step(const Common::Point &delta) {
	_mapPos += delta;
	byte &visited = _currentMap->_visited[_mapPos.y * MAP_W + _mapPos.x];
	if (!visited) {
		visited = VISITED_NORMAL;
	}

	int section = 0, id = 0;
	if (_mapPos.x < 0) {
		_mapPos.x = MAP_W - 1;
		id = _currentMap->dataWord(MAP_WEST_EXIT_ID);
		section = _currentMap->dataByte(MAP_WEST_EXIT_SECTION);
	} else if (_mapPos.x >= MAP_W) {
		_mapPos.x = 0;
		id = _currentMap->dataWord(MAP_EAST_EXIT_ID);
		section = _currentMap->dataByte(MAP_EAST_EXIT_SECTION);
	} else if (_mapPos.y < 0) {
		_mapPos.y = MAP_H - 1;
		id = _currentMap->dataWord(MAP_SOUTH_EXIT_ID);
		section = _currentMap->dataByte(MAP_SOUTH_EXIT_SECTION);
	} else if (_mapPos.y >= MAP_H) {
		_mapPos.y = 0;
		id = _currentMap->dataWord(MAP_NORTH_EXIT_ID);
		section = _currentMap->dataByte(MAP_NORTH_EXIT_SECTION);
	} else {
		return;
	}

	changeMap(id, section);
}

void Maps::changeMap(uint16 id, byte section) {
	// At this point, a new map is being entered
	select(id, section);
	loadTiles();

	visitedTile();

	g_events->send("Game", GameMessage("UPDATE"));
}

void Maps::visitedTile() {
	byte &visited = _currentMap->_visited[_mapPos.y * MAP_W + _mapPos.x];
	if (!visited)
		visited = VISITED_NORMAL;
}

void Maps::clearSpecial() {
	_currentState &= ~CELL_SPECIAL;
	_currentMap->_states[_mapOffset] &= ~CELL_SPECIAL;
}

Common::Point Maps::getMoveDelta(byte mask) {
	switch (mask) {
	case DIRMASK_E:
		return Common::Point(1, 0);
	case DIRMASK_W:
		return Common::Point(-1, 0);
	case DIRMASK_S:
		return Common::Point(0, -1);
	default:
		return Common::Point(0, 1);
	}
}

void Maps::updateMasksOffsets() {
	switch (_forwardMask) {
	case DIRMASK_N:
		_leftMask = DIRMASK_W;
		_rightMask = DIRMASK_E;
		_backwardsMask = DIRMASK_S;

		_forwardOffset = MAP_W;
		_leftOffset = -1;
		_rightOffset = 1;
		_backwardsOffset = -MAP_W;
		break;

	case DIRMASK_E:
		_leftMask = DIRMASK_N;
		_rightMask = DIRMASK_S;
		_backwardsMask = DIRMASK_W;

		_forwardOffset = 1;
		_leftOffset = MAP_W;
		_rightOffset = -MAP_W;
		_backwardsOffset = -1;
		break;

	case DIRMASK_S:
		_leftMask = DIRMASK_E;
		_rightMask = DIRMASK_W;
		_backwardsMask = DIRMASK_N;

		_forwardOffset = -MAP_W;
		_leftOffset = 1;
		_rightOffset = -1;
		_backwardsOffset = MAP_W;
		break;

	case DIRMASK_W:
		_leftMask = DIRMASK_S;
		_rightMask = DIRMASK_N;
		_backwardsMask = DIRMASK_E;

		_forwardOffset = -1;
		_leftOffset = -MAP_W;
		_rightOffset = MAP_W;
		_backwardsOffset = 1;
		break;

	default:
		break;
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
