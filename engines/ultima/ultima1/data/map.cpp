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

#include "common/file.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/data/map_dungeon.h"
#include "ultima/ultima1/data/tiles.h"
#include "ultima/ultima1/logic/city_castle_logic.h"
#include "ultima/ultima1/logic/dungeon_logic.h"
#include "ultima/ultima1/logic/mondain_logic.h"
#include "ultima/ultima1/logic/overworld_logic.h"
#include "ultima/ultima1/logic/space_map_logic.h"
#include "ultima/ultima1/views/dialog.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Data {

const char *DIRECTION_NAMES[5] = { nullptr, "West", "East", "North", "South" };
const char *SPACE_DIRECTION_NAMES[5] = { nullptr, "Counter-Clockwise", "Clockwise", "Thrust", "Retro" };

static const byte LOCATION_POS_X[LOCATION_COUNT] = {
	39, 66, 25, 46, 52, 18, 70, 64, 126, 128, 148, 115, 150, 121,
	150, 109, 42, 44, 64, 31, 66, 37, 66, 25, 128, 101, 142, 121,
	115, 149, 97, 103, 40, 32, 125, 114, 41, 30, 127, 135, 36, 69,
	96, 97, 13, 12, 131, 98, 48, 18, 53, 59, 29, 13, 62, 39,
	38, 130, 100, 124, 155, 147, 98, 109, 116, 136, 52, 32, 25, 14,
	63, 71, 40, 16, 46, 119, 149, 114, 108, 138, 154, 105, 128, 129
};

static const byte LOCATION_POS_Y[LOCATION_COUNT] = {
	39, 41, 61, 28, 63, 34, 63, 22, 36, 63, 22, 43, 49, 15,
	67, 61, 119, 92, 133, 112, 106, 140, 88, 94, 117, 119, 139, 106,
	141, 112, 141, 100, 38, 27, 37, 29, 118, 126, 116, 105, 9, 10,
	33, 66, 89, 122, 87, 88, 11, 13, 22, 29, 37, 43, 49, 60,
	68, 10, 15, 26, 35, 36, 45, 50, 56, 59, 96, 99, 105, 110,
	119, 120, 129, 140, 145, 89, 91, 100, 107, 115, 121, 127, 138, 146
};

// Indexed the same way as LOCATION_POS_X/Y (locationNum - 1) - cities
// (1-32), castles (33-40), pillars/sign posts (41-47), then dungeons (48-84)
const char *LOCATION_NAMES[LOCATION_COUNT] = {
	"Britian", "Moon", "Fawn", "Paws", "Montor", "Yew", "Tune", "Grey",
	"Arnold", "Linda", "Helen", "Owen", "John", "Gerry", "Wolf", "The Snake",
	"Nassau", "Clear Lagoon", "Stout", "Gauntlet", "Imagination", "Ponder",
	"Wealth", "Poor", "Gorlab", "Dextron", "Magic", "Wheeler", "Bulldozer",
	"The Brother", "Turtle", "Lost Friends",

	"The Castle of Lord British", "The Castle of the Lost King",
	"The Castle Barataria", "The Castle Rondorin", "The Castle of Olympus",
	"The Black Dragon's Castle", "The White Dragon's Castle",
	"The Castle of Shamino",

	"The Pillars of Protection", "The Tower of Knowledge",
	"The Pillars of the Argonauts", "The Pillar of Ozymandias",
	"The Sign Post", "The Southern Sign Post", "The Eastern Sign Post",

	"The Grave of the Lost Soul", "The Unholy Hole", "The Dungeon of Perinia",
	"The Dungeon of Montor", "The Mines of Mt. Drash", "Mondain's Gate to Hell",
	"The Lost Caverns", "The Dungeon of Doubt", "The Mines of Mt. Drash II",
	"Death's Awakening", "The Savage Place", "Scorpion Hole", "Advari's Hole",
	"The Dead Warrior's Fight", "The Horror of the Harpies", "The Labyrinth",
	"Where Hercules Died", "The Horror of the Harpies II", "The Gorgon Hole",
	"The Tramp of Doom", "The Viper's Pit", "The Long Death", "The End...",
	"The Viper's Pit II", "The Slow Death", "The Guild of Death",
	"The Metal Twister", "The Troll's Hole", "The Skull Smasher",
	"The Spine Breaker", "The Dungeon of Doom", "The Dead Cat's Life",
	"The Morbid Adventure", "Free Death Hole", "Dead Man's Walk",
	"The Dead Cat's Life II", "The Hole to Hades"
};

const char *CONTINENT_NAMES[4] = {
	"of Lord British",
	"of the Feudal Lords",
	"of the Dark Unknown",
	"of Danger and Despair"
};

// The Mondain encounter room. The original stores this top-to-bottom
// columns first, then left-to-right (MONDAIN_WIDTH columns of MONDAIN_HEIGHT
// bytes each); transposed here to left-to-right rows, top-to-bottom, for
// consistency with the overworld/city maps
static const byte MONDAIN_MAP[MONDAIN_HEIGHT][MONDAIN_WIDTH] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 0, 5, 5, 5, 0 },
	{ 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 0, 5, 0, 0, 0, 0, 0, 5, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0 },
	{ 0, 0, 1,20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0 },
	{ 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 0, 5, 0, 0, 0, 0, 0, 5, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 0, 5, 5, 5, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void Map::init() {
	// Load the overworld map
	Common::File ow;
	byte v;
	byte *offset;

	if (!ow.open("map.bin"))
		error("Could not open map.bin");
	_overworldMap.reserve(OVERWORLD_SIZE);

	for (int i = 0; i < OVERWORLD_SIZE / 2; ++i) {
		v = ow.readByte();
		_overworldMap.push_back(v >> 4);
		_overworldMap.push_back(v & 0xf);
	}

	ow.close();

	// Load the city/castle maps
	Common::File tc;
	if (!tc.open("tcd.bin"))
		error("Could not open tcd.bin");

	for (int i = 0; i < 10; ++i) {
		auto &m = _cityMap[i];
		m.resize(CITY_SIZE);

		// Individual cities are top-to-bottom first, then left-to-right. For consistency
		// with the overworld, we switch them to left-to-right, top-to-bottom
		for (int x = 0; x < CITY_WIDTH; ++x) {
			offset = &m[x];

			for (int y = 0; y < CITY_HEIGHT; ++y, offset += CITY_WIDTH)
				*offset = tc.readByte();
		}
	}

	// Copy the Mondain map
	resetMondainMap();

	// Load tiles
	loadTiles("egatiles.bin", _overworldTiles, OVERWORLD_TILES_COUNT, 16);
	loadTiles("egatown.bin", _cityTiles, CITY_TILES_COUNT, 8);

	// Tile 0 is a blank empty floor tile, followed by 19 tiles, and tile 20 is duplicated from egatiles.bin
	_mondainTiles[0].create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
	loadTiles("egamond.bin", &_mondainTiles[1], MONDAIN_TILES_COUNT, 16);
	_mondainTiles[20] = _overworldTiles[TILE_PLAYER];

	loadShipTiles("egaspace.bin", _spaceShipTiles, SPACE_SHIP_TILES_COUNT);
}

void Map::resetMondainMap() {
	Common::copy(&MONDAIN_MAP[0][0], &MONDAIN_MAP[0][0] + MONDAIN_WIDTH * MONDAIN_HEIGHT, &_mondainMap[0][0]);
}

void Map::load(int mapNum) {
	_G(savegame)._mapNum = mapNum;

	if (mapNum == MAP_OVERWORLD) {
		_mapWidth = OVERWORLD_WIDTH;
		_mapHeight = OVERWORLD_HEIGHT;
		_outsideMapTile = 0xff;
		_tiles = _overworldTiles;

	} else if (mapNum == MAP_MONDAIN) {
		_mapWidth = MONDAIN_WIDTH;
		_mapHeight = MONDAIN_HEIGHT;
		_tiles = _mondainTiles;

	} else {
		_mapWidth = CITY_WIDTH;
		_mapHeight = CITY_HEIGHT;
		_outsideMapTile = CTILE_BLANK;
		_tiles = _cityTiles;
	}

	// Set up copies of the map position and player tile to use
	_playerTileId = TILE_PLAYER;

	// Set up logic handler for the mode and which view it'll be using
	Common::String viewName;
	if (mapNum == MAP_SPACE) {
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::SpaceMapLogic());
		viewName = "SpaceMap";
	} else if (mapNum == MAP_MONDAIN) {
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::MondainLogic());
		viewName = "MondainMap";
	} else if (mapNum >= 49) {
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::DungeonLogic());
		viewName = "DungeonMap";
	} else if (mapNum == MAP_OVERWORLD) {
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::OverworldLogic());
		viewName = "OverworldMap";
	} else {
		if (mapNum < 33)
			_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::CityLogic());
		else
			_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::CastleLogic());

		viewName = "LocationMap";
	}

	// Set up the map row pointers
	_mapRows.clear();
	switch (_mapType) {
	case MAPTYPE_OVERWORLD:
		_mapRows.reserve(OVERWORLD_HEIGHT);
		for (int y = 0; y < OVERWORLD_HEIGHT; ++y)
			_mapRows.push_back(Row(this, &_overworldMap[y * OVERWORLD_WIDTH]));
		break;

	case MAPTYPE_DUNGEON:
		_G(dungeon).generateDungeonLevel();
		_mapRows.clear();		// Dungeon doesn't use this map
		break;

	case MAPTYPE_MONDAIN:
		_mapRows.reserve(MONDAIN_HEIGHT);
		for (int y = 0; y < MONDAIN_HEIGHT; ++y)
			_mapRows.push_back(Row(this, &_mondainMap[y][0]));
		break;

	case MAPTYPE_SPACE:
		_mapRows.clear();		// Outer space doesn't use this map
		break;

	default:
		_mapRows.reserve(CITY_HEIGHT + 1);		// One extra row for out-of-bounds y access
		for (int y = 0; y < CITY_HEIGHT; ++y)
			_mapRows.push_back(Row(this, &_cityMap[_mapStyle][y * CITY_WIDTH]));
		_mapRows.push_back(Row(this, nullptr));
		break;
	}

	// Outer space uses its own base game frame (its own stats corner); make
	// sure the right one - SpaceGame vs Game - is the active base before the
	// map view goes on top of it
	bool isSpaceView = (viewName == "SpaceMap" || viewName == "SpaceCockpit");
	const char *baseViewName = isSpaceView ? "SpaceGame" : "Game";
	if (!g_engine->isPresent(baseViewName))
		g_engine->replaceView(baseViewName, true);

	// If the focused view isn't already a dialog under the game frame, then reset to the game
	// frame and draw it immediately
	Views::Dialog *view = dynamic_cast<Views::Dialog *>(g_engine->focusedView());
	if (view == nullptr) {
		g_engine->replaceView(baseViewName, true);
		g_engine->focusedView()->draw();

		// Now add the appropriate map view on top of it
		g_engine->addView(viewName);
	} else {
		// Already showing a game dialog, so just replace the current one
		g_engine->replaceView(viewName, false);
	}
}

int Map::getMapTile(int x, int y) const {
	int tile = (*this)[y][x];

	// Raw overworld map data skips the castle/city flag animation frames
	// (TILE_CASTLE2/TILE_CITY2), which are only ever substituted in at
	// draw time and never stored in the map itself. This doesn't apply
	// to city/castle map data, which uses a completely different,
	// contiguous tile ID space (see the Town.bin tile table)
	if (_G(savegame)._mapNum == MAP_OVERWORLD && tile > TILE_CASTLE1) {
		++tile;
		if (tile == TILE_CITY2)
			++tile;
	}

	return tile;
}

int Map::getTileAt(int x, int y, int creatureIndex) const {
	int entityIndex;

	if (_mapType == MAPTYPE_OVERWORLD) {
		entityIndex = _G(savegame).getOverworldEntityAt(x, y, creatureIndex);
		if (entityIndex >= 0)
			return _G(savegame)._overworldEntities[entityIndex]._type;
	} else {
		entityIndex = _G(savegame).getLocationEntityAt(x, y);
		if (entityIndex >= 0)
			return _G(savegame)._locationEntities[entityIndex]._type;
	}

	return _G(map).getMapTile(x, y);
}

int Map::getContinentAt(int x, int y) const {
	return (y > 77 ? 2 : 0) + (x > 83 ? 1 : 0);
}

int Map::getLocationAt(int x, int y) const {
	for (int i = 0; i < LOCATION_COUNT; ++i) {
		if (x == LOCATION_POS_X[i] && y == LOCATION_POS_Y[i])
			return i + 1;
	}

	return 0;
}

bool Map::mapRangeCheckX(int xp) const {
	if (xp > 83)
		xp -= 84;

	return !(xp >= 159 || xp <= 8);
}

bool Map::mapRangeCheckY(int yp) const {
	if (yp > 77)
		yp -= 78;

	return !(yp >= 150 || yp <= 5);
}

} // namespace Data
} // namespace Ultima1
} // namespace Ultima
