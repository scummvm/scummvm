/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/map/maploader.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/conversation/dialogueloader.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/maploader.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/xml_map.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/image.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "common/file.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

MapLoaders *g_mapLoaders;

MapLoaders::MapLoaders() {
	g_mapLoaders = this;

	(*this)[Map::CITY] = new CityMapLoader();
	(*this)[Map::SHRINE] = new ConMapLoader();
	(*this)[Map::DUNGEON] = new DngMapLoader();
	(*this)[Map::WORLD] = new WorldMapLoader();
	(*this)[Map::COMBAT] = new ConMapLoader();
	(*this)[Map::XML] = new XMLMapLoader();
}

MapLoaders::~MapLoaders() {
	// Free the loaders
	for (iterator it = begin(); it != end(); ++it)
		delete it->_value;

	g_mapLoaders = nullptr;
}

MapLoader *MapLoaders::getLoader(Map::Type type) {
	if (find(type) == end())
		return nullptr;

	return (*this)[type];
}

/*-------------------------------------------------------------------*/

bool MapLoader::loadData(Map *map, Common::SeekableReadStream &f) {
	uint x, xch, y, ych;

	// Allocate the space we need for the map data
	map->_data.clear();
	map->_data.resize(map->_height * map->_width);

	if (map->_chunkHeight == 0)
		map->_chunkHeight = map->_height;
	if (map->_chunkWidth == 0)
		map->_chunkWidth = map->_width;

	uint32 total = 0;
	f.seek(map->_offset, SEEK_CUR);

	for (ych = 0; ych < (map->_height / map->_chunkHeight); ++ych) {
		for (xch = 0; xch < (map->_width / map->_chunkWidth); ++xch) {
			if (isChunkCompressed(map, ych * map->_chunkWidth + xch)) {
				MapTile water = map->_tileSet->getByName("sea")->getId();
				for (y = 0; y < map->_chunkHeight; ++y) {
					for (x = 0; x < map->_chunkWidth; ++x) {
						map->_data[x + (y * map->_width) + (xch * map->_chunkWidth) + (ych * map->_chunkHeight * map->_width)] = water;
					}
				}
			} else {
				for (y = 0; y < map->_chunkHeight; ++y) {
					for (x = 0; x < map->_chunkWidth; ++x) {
						int c = f.readByte();
						if (c == EOF)
							return false;

						uint32 s = g_system->getMillis();
						MapTile mt = map->translateFromRawTileIndex(c);
						total += g_system->getMillis() - s;

						map->_data[x + (y * map->_width) + (xch * map->_chunkWidth) + (ych * map->_chunkHeight * map->_width)] = mt;
					}
				}
			}
		}
	}

	return true;
}

bool MapLoader::isChunkCompressed(Map *map, int chunk) {
	CompressedChunkList::iterator i;

	for (i = map->_compressedChunks.begin(); i != map->_compressedChunks.end(); i++) {
		if (chunk == *i)
			return true;
	}
	return false;
}

/*-------------------------------------------------------------------*/

bool CityMapLoader::load(Map *map) {
	City *city = dynamic_cast<City *>(map);
	assert(city);

	uint i, j;
	Person *people[CITY_MAX_PERSONS];
	Dialogue *dialogues[CITY_MAX_PERSONS];
	DialogueLoader *dlgLoader = DialogueLoaders::getLoader("application/x-u4tlk");

	Common::File ult, tlk;
	if (!ult.open(city->_fname) || !tlk.open(city->_tlkFname))
		error("unable to load map data");

	// The map must be 32x32 to be read from an .ULT file
	assertMsg(city->_width == CITY_WIDTH, "map width is %d, should be %d", city->_width, CITY_WIDTH);
	assertMsg(city->_height == CITY_HEIGHT, "map height is %d, should be %d", city->_height, CITY_HEIGHT);

	if (!loadData(city, ult))
		return false;

	// Properly construct people for the city
	for (i = 0; i < CITY_MAX_PERSONS; i++)
		people[i] = new Person(map->translateFromRawTileIndex(ult.readByte()));

	for (i = 0; i < CITY_MAX_PERSONS; i++)
		people[i]->getStart().x = ult.readByte();

	for (i = 0; i < CITY_MAX_PERSONS; i++)
		people[i]->getStart().y = ult.readByte();

	for (i = 0; i < CITY_MAX_PERSONS; i++)
		people[i]->setPrevTile(map->translateFromRawTileIndex(ult.readByte()));

	for (i = 0; i < CITY_MAX_PERSONS * 2; i++)
		ult.readByte();           // Read redundant startx/starty

	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		byte c = ult.readByte();
		if (c == 0)
			people[i]->setMovementBehavior(MOVEMENT_FIXED);
		else if (c == 1)
			people[i]->setMovementBehavior(MOVEMENT_WANDER);
		else if (c == 0x80)
			people[i]->setMovementBehavior(MOVEMENT_FOLLOW_AVATAR);
		else if (c == 0xFF)
			people[i]->setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
		else
			return false;
	}

	byte conv_idx[CITY_MAX_PERSONS];
	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		conv_idx[i] = ult.readByte();
	}

	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		people[i]->getStart().z = 0;
	}

	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		dialogues[i] = dlgLoader->load(&tlk);

		if (!dialogues[i])
			break;

		/*
		 * Match up dialogues with their respective people
		 */
		bool found = false;
		for (j = 0; j < CITY_MAX_PERSONS; j++) {
			if (conv_idx[j] == i + 1) {
				people[j]->setDialogue(dialogues[i]);
				found = true;
			}
		}
		/*
		 * if the dialogue doesn't match up with a person, attach it
		 * to the city; Isaac the ghost in Skara Brae is handled like
		 * this
		 */
		if (!found) {
			city->_extraDialogues.push_back(dialogues[i]);
		}
	}

	/*
	 * Assign roles to certain people
	 */
	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		PersonRoleList::iterator current;

		for (current = city->_personRoles.begin(); current != city->_personRoles.end(); current++) {
			if ((unsigned)(*current)->_id == (i + 1)) {
				if ((*current)->_role == NPC_LORD_BRITISH)
					people[i]->setDialogue(DialogueLoaders::getLoader("application/x-u4lbtlk")->load(nullptr));
				else if ((*current)->_role == NPC_HAWKWIND)
					people[i]->setDialogue(DialogueLoaders::getLoader("application/x-u4hwtlk")->load(nullptr));
				people[i]->setNpcType(static_cast<PersonNpcType>((*current)->_role));
			}
		}
	}

	/**
	 * Add the people to the city structure
	 */
	for (i = 0; i < CITY_MAX_PERSONS; i++) {
		if (people[i]->getTile() != 0)
			city->_persons.push_back(people[i]);
		else
			delete people[i];
	}

	return true;
}

/*-------------------------------------------------------------------*/

bool ConMapLoader::load(Map *map) {
	int i;

	Common::File con;
	if (!con.open(map->_fname))
		error("unable to load map data");

	// The map must be 11x11 to be read from an .CON file
	assertMsg(map->_width == CON_WIDTH, "map width is %d, should be %d", map->_width, CON_WIDTH);
	assertMsg(map->_height == CON_HEIGHT, "map height is %d, should be %d", map->_height, CON_HEIGHT);

	if (map->_type != Map::SHRINE) {
		CombatMap *cm = getCombatMap(map);

		for (i = 0; i < AREA_CREATURES; i++)
			cm->creature_start[i] = MapCoords(con.readByte());

		for (i = 0; i < AREA_CREATURES; i++)
			cm->creature_start[i].y = con.readByte();

		for (i = 0; i < AREA_PLAYERS; i++)
			cm->player_start[i] = MapCoords(con.readByte());

		for (i = 0; i < AREA_PLAYERS; i++)
			cm->player_start[i].y = con.readByte();

		con.seek(16L, SEEK_CUR);
	}

	if (!loadData(map, con))
		return false;

	return true;
}

/*-------------------------------------------------------------------*/

bool DngMapLoader::load(Map *map) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(map);
	assert(dungeon);

	Common::File dng;
	if (!dng.open(dungeon->_fname))
		error("unable to load map data");

	// The map must be 11x11 to be read from an .CON file
	assertMsg(dungeon->_width == DNG_WIDTH, "map width is %d, should be %d", dungeon->_width, DNG_WIDTH);
	assertMsg(dungeon->_height == DNG_HEIGHT, "map height is %d, should be %d", dungeon->_height, DNG_HEIGHT);

	// Load the dungeon map
	uint i, j;
	for (i = 0; i < (DNG_HEIGHT * DNG_WIDTH * dungeon->_levels); i++) {
		byte mapData = dng.readByte();
		MapTile tile = map->translateFromRawTileIndex(mapData);

		// Determine what type of tile it is
		dungeon->_data.push_back(tile);
		dungeon->_dataSubTokens.push_back(mapData % 16);
	}

	// Read in the dungeon rooms
	// FIXME: needs a cleanup function to free this memory later
	dungeon->_rooms = new DngRoom[dungeon->_nRooms];

	for (i = 0; i < dungeon->_nRooms; i++) {
		byte room_tiles[121];

		for (j = 0; j < DNGROOM_NTRIGGERS; j++) {
			int tmp;

			dungeon->_rooms[i]._triggers[j]._tile = g_tileMaps->get("base")->translate(dng.readByte())._id;

			tmp = dng.readByte();
			if (tmp == EOF)
				return false;
			dungeon->_rooms[i]._triggers[j].x = (tmp >> 4) & 0x0F;
			dungeon->_rooms[i]._triggers[j].y = tmp & 0x0F;

			tmp = dng.readByte();
			if (tmp == EOF)
				return false;
			dungeon->_rooms[i]._triggers[j]._changeX1 = (tmp >> 4) & 0x0F;
			dungeon->_rooms[i]._triggers[j]._changeY1 = tmp & 0x0F;

			tmp = dng.readByte();
			if (tmp == EOF)
				return false;
			dungeon->_rooms[i]._triggers[j].changeX2 = (tmp >> 4) & 0x0F;
			dungeon->_rooms[i]._triggers[j].changeY2 = tmp & 0x0F;
		}

		dungeon->_rooms[i].load(dng);
		dng.read(room_tiles, sizeof(room_tiles));
		dng.read(dungeon->_rooms[i]._buffer, sizeof(dungeon->_rooms[i]._buffer));

		// Translate each creature tile to a tile id
		for (j = 0; j < sizeof(dungeon->_rooms[i]._creatureTiles); j++)
			dungeon->_rooms[i]._creatureTiles[j] = g_tileMaps->get("base")->translate(dungeon->_rooms[i]._creatureTiles[j])._id;

		// Translate each map tile to a tile id
		for (j = 0; j < sizeof(room_tiles); j++)
			dungeon->_rooms[i]._mapData.push_back(g_tileMaps->get("base")->translate(room_tiles[j]));

		//
		// dungeon room fixup
		//
		if (map->_id == MAP_HYTHLOTH) {
			if (i == 0x7) {
				dungeon->_rooms[i].hythlothFix7();
			} else if (i == 0x9) {
				dungeon->_rooms[i].hythlothFix9();
			}
		}
	}

	dungeon->_roomMaps = new CombatMap *[dungeon->_nRooms];
	for (i = 0; i < dungeon->_nRooms; i++)
		initDungeonRoom(dungeon, i);

	return true;
}

/*-------------------------------------------------------------------*/

void DngMapLoader::initDungeonRoom(Dungeon *dng, int room) {
	dng->_roomMaps[room] = dynamic_cast<CombatMap *>(mapMgr->initMap(Map::COMBAT));

	dng->_roomMaps[room]->_id = 0;
	dng->_roomMaps[room]->_borderBehavior = Map::BORDER_FIXED;
	dng->_roomMaps[room]->_width = dng->_roomMaps[room]->_height = 11;
	dng->_roomMaps[room]->_data = dng->_rooms[room]._mapData; // Load map data
	dng->_roomMaps[room]->_music = Music::COMBAT;
	dng->_roomMaps[room]->_type = Map::COMBAT;
	dng->_roomMaps[room]->_flags |= NO_LINE_OF_SIGHT;
	dng->_roomMaps[room]->_tileSet = g_tileSets->get("base");
}

/*-------------------------------------------------------------------*/

bool WorldMapLoader::load(Map *map) {
	Common::File world;
	if (!world.open(map->_fname))
		error("unable to load map data");

	if (!loadData(map, world))
		return false;

	// Check for any tile overrides for the portals
	for (uint idx = 0; idx < map->_portals.size(); ++idx) {
		const Portal *p = map->_portals[idx];
		if (p->_tile != -1) {
			MapTile mt = map->translateFromRawTileIndex(p->_tile);
			map->_data[p->_coords.x + p->_coords.y * map->_width] = mt;
		}
	}

	return true;
}

/*-------------------------------------------------------------------*/

bool XMLMapLoader::load(Map *map) {
	XMLMap *xmlMap = dynamic_cast<XMLMap *>(map);
	assert(xmlMap);
	Common::String text = xmlMap->_tilesText;
	text.trim();

	// Allocate the space we need for the map data
	map->_data.clear();
	map->_data.resize(map->_width * map->_height);

	// Split up the text lines
	Common::StringArray lines, cols;	
	split(text, lines, '\n');
	assert(lines.size() == map->_height);

	// Iterate through the lines
	for (uint y = 0; y < map->_height; ++y) {
		text = lines[y];
		text.trim();
		split(text, cols, ',');
		assert(cols.size() == map->_width);

		for (uint x = 0; x < map->_width; ++x) {
			int id = atoi(cols[x].c_str());
			MapTile mt = map->translateFromRawTileIndex(id);
			map->_data[x + y * map->_width] = mt;
		}
	}

	return true;
}

void XMLMapLoader::split(const Common::String &text, Common::StringArray &values, char c) {
	values.clear();

	Common::String str = text;
	size_t pos;
	while ((pos = str.findFirstOf(c)) != Common::String::npos) {
		values.push_back(Common::String(str.c_str(), pos));
		str = Common::String(str.c_str() + pos + 1);
	}

	values.push_back(str);
}

} // End of namespace Ultima4
} // End of namespace Ultima
