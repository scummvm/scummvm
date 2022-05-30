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
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/gfx/dta.h"
#include "mm/mm1/gfx/screen_decoder.h"
#include "mm/mm1/events.h"
#include "common/system.h"

namespace MM {
namespace MM1 {
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


Maps::Maps() :
		_map00(this), _map01(this), _map02(this), _map03(this),
		_map04(this), _map05(this), _map06(this), _map07(this),
		_map08(this), _map09(this), _map10(this), _map11(this),
		_map12(this), _map13(this), _map14(this), _map15(this),
		_map16(this), _map17(this), _map18(this), _map19(this),
		_map20(this), _map21(this), _map22(this), _map23(this),
		_map24(this), _map25(this), _map26(this), _map27(this),
		_map28(this), _map29(this), _map30(this), _map31(this),
		_map32(this), _map33(this), _map34(this), _map35(this),
		_map36(this), _map37(this), _map38(this), _map39(this),
		_map40(this), _map41(this), _map42(this), _map43(this),
		_map44(this), _map45(this), _map46(this), _map47(this),
		_map48(this), _map49(this), _map50(this), _map51(this),
		_map52(this), _map53(this), _map54(this) {
	Common::fill(&_data1[0], &_data1[32], 0);
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
	g_events->msgGame(GameMessage("DISPLAY"));
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
}

void Maps::town15setup() {
	_mapDirectionMask = DIRMASK_N;
	_val1 = 3;
	_val2 = 0x30;
	_val3 = 0xC;
	_val4 = 0x10;
	_val5 = 0xff;
	_val6 = 1;
	_val7 = 0xf0;
}

void Maps::town23setup() {
	_mapDirectionMask = DIRMASK_W;
	_val1 = 0xC;
	_val2 = 0xC0;
	_val3 = 0x30;
	_val4 = 0xff;
	_val5 = 0xf0;
	_val6 = 0x10;
	_val7 = 1;
}

void Maps::town4setup() {
	_mapDirectionMask = DIRMASK_E;
	_val1 = 0xc0;
	_val2 = 0xC;
	_val3 = 3;
	_val4 = 1;
	_val5 = 0x10;
	_val6 = 0xf0;
	_val7 = 0xff;
}

uint Maps::getIndex(uint16 id, byte section) {
	uint idx = LOOKUPS_START[section / 2];

	// Find map by Id
	for (; id != _maps[idx]->getId(); ++idx) {
		assert(idx < _maps.size());
	}

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

	// ***DEBUG*** - Display the first tile of stream
	Graphics::Screen &scr = *g_events->getScreen();
	scr.blitFrom(_tiles[_loadSection - 1].front());
	scr.update();
	Common::Event e;
	g_system->getEventManager()->pollEvent(e);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
