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
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/maploader.h"
#include "ultima/ultima4/city.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/conversation.h"
#include "ultima/ultima4/dialogueloader.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeon.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/filesystem.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/maploader.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/portal.h"
#include "ultima/ultima4/tilemap.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/utils.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imagemgr.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

Std::map<Map::Type, MapLoader *, MapType_Hash> *MapLoader::loaderMap = NULL;

MapLoader *CityMapLoader::_instance = MapLoader::registerLoader(new CityMapLoader, Map::CITY);
MapLoader *ConMapLoader::_instance = MapLoader::registerLoader(MapLoader::registerLoader(new ConMapLoader, Map::COMBAT), Map::SHRINE);
MapLoader *DngMapLoader::_instance = MapLoader::registerLoader(new DngMapLoader, Map::DUNGEON);
MapLoader *WorldMapLoader::_instance = MapLoader::registerLoader(new WorldMapLoader, Map::WORLD);

/**
 * Gets a map loader for the given map type.
 */
MapLoader *MapLoader::getLoader(Map::Type type) {
    ASSERT(loaderMap != NULL, "ImageLoader::getLoader loaderMap not initialized");
    if (loaderMap->find(type) == loaderMap->end())
        return NULL;
    return (*loaderMap)[type];
}

/**
 * Registers a loader for the given map type.
 */
MapLoader *MapLoader::registerLoader(MapLoader *loader, Map::Type type) {
    if (loaderMap == NULL)
        loaderMap = new Std::map<Map::Type, MapLoader *, MapType_Hash>();

    if (loaderMap->find(type) != loaderMap->end())
        errorFatal("map loader already registered for type %d", type);

    (*loaderMap)[type] = loader;
    return loader;
}

/**
 * Loads raw data from the given file.  
 */
bool MapLoader::loadData(Map *map, U4FILE *f) {
    unsigned int x, xch, y, ych;

    /* allocate the space we need for the map data */
	map->_data.clear();
	map->_data.resize(map->_height * map->_width);

    if (map->_chunkHeight == 0)
        map->_chunkHeight = map->_height;
    if (map->_chunkWidth == 0)
        map->_chunkWidth = map->_width;

    uint32 total = 0;
    u4fseek(f, map->_offset, SEEK_CUR);

    for(ych = 0; ych < (map->_height / map->_chunkHeight); ++ych) {
        for(xch = 0; xch < (map->_width / map->_chunkWidth); ++xch) {
            if (isChunkCompressed(map, ych * map->_chunkWidth + xch)) {
                MapTile water = map->_tileset->getByName("sea")->getId();
                for(y = 0; y < map->_chunkHeight; ++y) {
                    for(x = 0; x < map->_chunkWidth; ++x) {
                        map->_data[x + (y * map->_width) + (xch * map->_chunkWidth) + (ych * map->_chunkHeight * map->_width)] = water;
                    }
                }
            }
            else {
                for(y = 0; y < map->_chunkHeight; ++y) {
                    for(x = 0; x < map->_chunkWidth; ++x) {                    
                        int c = u4fgetc(f);
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

/**
 * Load city data from 'ult' and 'tlk' files.
 */
bool CityMapLoader::load(Map *map) {
    City *city = dynamic_cast<City*>(map);

    unsigned int i, j;
    Person *people[CITY_MAX_PERSONS];    
    Dialogue *dialogues[CITY_MAX_PERSONS];
    DialogueLoader *dlgLoader = DialogueLoader::getLoader("application/x-u4tlk");

    U4FILE *ult = u4fopen(city->_fname);
    U4FILE *tlk = u4fopen(city->_tlkFname);
    if (!ult || !tlk)
        errorFatal("unable to load map data");

    /* the map must be 32x32 to be read from an .ULT file */
    ASSERT(city->_width == CITY_WIDTH, "map width is %d, should be %d", city->_width, CITY_WIDTH);
    ASSERT(city->_height == CITY_HEIGHT, "map height is %d, should be %d", city->_height, CITY_HEIGHT);

    if (!loadData(city, ult))
        return false;

    /* Properly construct people for the city */       
    for (i = 0; i < CITY_MAX_PERSONS; i++)
        people[i] = new Person(map->translateFromRawTileIndex(u4fgetc(ult)));

    for (i = 0; i < CITY_MAX_PERSONS; i++)
        people[i]->getStart().x = u4fgetc(ult);

    for (i = 0; i < CITY_MAX_PERSONS; i++)
        people[i]->getStart().y = u4fgetc(ult);

    for (i = 0; i < CITY_MAX_PERSONS; i++)
        people[i]->setPrevTile(map->translateFromRawTileIndex(u4fgetc(ult)));

    for (i = 0; i < CITY_MAX_PERSONS * 2; i++)
        u4fgetc(ult);           /* read redundant startx/starty */

    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        unsigned char c = u4fgetc(ult);
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

    unsigned char conv_idx[CITY_MAX_PERSONS];
    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        conv_idx[i] = u4fgetc(ult);
    }

    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        people[i]->getStart().z = 0;        
    }

    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        dialogues[i] = dlgLoader->load(tlk);

        if (!dialogues[i])
            break;

        /*
         * Match up dialogues with their respective people
         */
        bool found = false;
        for (j = 0; j < CITY_MAX_PERSONS; j++) {
            if (conv_idx[j] == i+1) {
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
                    people[i]->setDialogue(DialogueLoader::getLoader("application/x-u4lbtlk")->load(NULL));
                else if ((*current)->_role == NPC_HAWKWIND)
                    people[i]->setDialogue(DialogueLoader::getLoader("application/x-u4hwtlk")->load(NULL));
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

    u4fclose(ult);
    u4fclose(tlk);

    return true;
}

/**
 * Loads a combat map from the 'con' file
 */
bool ConMapLoader::load(Map *map) {
    int i;

    U4FILE *con = u4fopen(map->_fname);
    if (!con)
        errorFatal("unable to load map data");

    /* the map must be 11x11 to be read from an .CON file */
    ASSERT(map->_width == CON_WIDTH, "map width is %d, should be %d", map->_width, CON_WIDTH);
    ASSERT(map->_height == CON_HEIGHT, "map height is %d, should be %d", map->_height, CON_HEIGHT);

    if (map->_type != Map::SHRINE) {
        CombatMap *cm = getCombatMap(map);

        for (i = 0; i < AREA_CREATURES; i++)
            cm->creature_start[i] = MapCoords(u4fgetc(con));        

        for (i = 0; i < AREA_CREATURES; i++)
            cm->creature_start[i].y = u4fgetc(con);

        for (i = 0; i < AREA_PLAYERS; i++)
            cm->player_start[i] = MapCoords(u4fgetc(con));

        for (i = 0; i < AREA_PLAYERS; i++)
            cm->player_start[i].y = u4fgetc(con);

        u4fseek(con, 16L, SEEK_CUR);
    }

    if (!loadData(map, con))
        return false;

    u4fclose(con);

    return true;
}

/**
 * Loads a dungeon map from the 'dng' file
 */
bool DngMapLoader::load(Map *map) {
    Dungeon *dungeon = dynamic_cast<Dungeon*>(map);    

    U4FILE *dng = u4fopen(dungeon->_fname);
    if (!dng)
        errorFatal("unable to load map data");

    /* the map must be 11x11 to be read from an .CON file */
    ASSERT(dungeon->_width == DNG_WIDTH, "map width is %d, should be %d", dungeon->_width, DNG_WIDTH);
    ASSERT(dungeon->_height == DNG_HEIGHT, "map height is %d, should be %d", dungeon->_height, DNG_HEIGHT);

    /* load the dungeon map */
    unsigned int i, j;
    for (i = 0; i < (DNG_HEIGHT * DNG_WIDTH * dungeon->_levels); i++) {
        unsigned char mapData = u4fgetc(dng);
        MapTile tile = map->translateFromRawTileIndex(mapData);
        
        /* determine what type of tile it is */
        dungeon->_data.push_back(tile);
        dungeon->_dataSubTokens.push_back(mapData % 16);
    }

    /* read in the dungeon rooms */
    /* FIXME: needs a cleanup function to free this memory later */
    dungeon->_rooms = new DngRoom[dungeon->_nRooms];

    for (i = 0; i < dungeon->_nRooms; i++) {
        unsigned char room_tiles[121];

        for (j = 0; j < DNGROOM_NTRIGGERS; j++) {
            int tmp;

            dungeon->_rooms[i]._triggers[j]._tile = TileMap::get("base")->translate(u4fgetc(dng))._id;

            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->_rooms[i]._triggers[j].x = (tmp >> 4) & 0x0F;
            dungeon->_rooms[i]._triggers[j].y = tmp & 0x0F;

            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->_rooms[i]._triggers[j]._changeX1 = (tmp >> 4) & 0x0F;
            dungeon->_rooms[i]._triggers[j]._changeY1 = tmp & 0x0F;
            
            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->_rooms[i]._triggers[j].changeX2 = (tmp >> 4) & 0x0F;
            dungeon->_rooms[i]._triggers[j].changeY2 = tmp & 0x0F;
        }

        u4fread(dungeon->_rooms[i]._creatureTiles, sizeof(dungeon->_rooms[i]._creatureTiles), 1, dng);
        u4fread(dungeon->_rooms[i]._creatureStartX, sizeof(dungeon->_rooms[i]._creatureStartX), 1, dng);
        u4fread(dungeon->_rooms[i]._creatureStartY, sizeof(dungeon->_rooms[i]._creatureStartY), 1, dng);
        u4fread(dungeon->_rooms[i]._partyNorthStartX, sizeof(dungeon->_rooms[i]._partyNorthStartX), 1, dng);
        u4fread(dungeon->_rooms[i]._partyNorthStartY, sizeof(dungeon->_rooms[i]._partyNorthStartY), 1, dng);
        u4fread(dungeon->_rooms[i]._partyEastStartX, sizeof(dungeon->_rooms[i]._partyEastStartX), 1, dng);
        u4fread(dungeon->_rooms[i]._partyEastStartY, sizeof(dungeon->_rooms[i]._partyEastStartY), 1, dng);
        u4fread(dungeon->_rooms[i]._partySouthStartX, sizeof(dungeon->_rooms[i]._partySouthStartX), 1, dng);
        u4fread(dungeon->_rooms[i]._partySouthStartY, sizeof(dungeon->_rooms[i]._partySouthStartY), 1, dng);
        u4fread(dungeon->_rooms[i]._partyWestStartX, sizeof(dungeon->_rooms[i]._partyWestStartX), 1, dng);
        u4fread(dungeon->_rooms[i]._partyWestStartY, sizeof(dungeon->_rooms[i]._partyWestStartY), 1, dng);
        u4fread(room_tiles, sizeof(room_tiles), 1, dng);
        u4fread(dungeon->_rooms[i]._buffer, sizeof(dungeon->_rooms[i]._buffer), 1, dng);

        /* translate each creature tile to a tile id */
        for (j = 0; j < sizeof(dungeon->_rooms[i]._creatureTiles); j++)
            dungeon->_rooms[i]._creatureTiles[j] = TileMap::get("base")->translate(dungeon->_rooms[i]._creatureTiles[j])._id;

        /* translate each map tile to a tile id */
        for (j = 0; j < sizeof(room_tiles); j++)
            dungeon->_rooms[i]._mapData.push_back(TileMap::get("base")->translate(room_tiles[j]));

        //
        // dungeon room fixup
        //
        if (map->_id == MAP_HYTHLOTH)
        {
            // A couple rooms in hythloth have NULL player start positions,
            // which causes the entire party to appear in the upper-left
            // tile when entering the dungeon room.
            //
            // Also, one dungeon room is apparently supposed to be connected
            // to another, although the the connection does not exist in the
            // DOS U4 dungeon data file.  This was fixed by removing a few
            // wall tiles, and relocating a chest and the few monsters around
            // it to the center of the room.
            //
            if (i == 0x7)
            {
                // update party start positions when entering from the east
                const unsigned char x1[8] = { 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA },
                                    y1[8] = { 0x3, 0x2, 0x3, 0x2, 0x1, 0x3, 0x2, 0x1 };
                memcpy(dungeon->_rooms[i]._partyEastStartX, x1, sizeof(x1));
                memcpy(dungeon->_rooms[i]._partyEastStartY, y1, sizeof(y1));

                // update party start positions when entering from the south
                const unsigned char x2[8] = { 0x3, 0x2, 0x3, 0x2, 0x1, 0x3, 0x2, 0x1 },
                                    y2[8] = { 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA };
                memcpy(dungeon->_rooms[i]._partySouthStartX, x2, sizeof(x2));
                memcpy(dungeon->_rooms[i]._partySouthStartY, y2, sizeof(y2));
            }
            else if (i == 0x9)
            {
                // update the starting position of monsters 7, 8, and 9
                const unsigned char x1[3] = { 0x4, 0x6, 0x5 },
                                    y1[3] = { 0x5, 0x5, 0x6 };
                memcpy(dungeon->_rooms[i]._creatureStartX+7, x1, sizeof(x1));
                memcpy(dungeon->_rooms[i]._creatureStartY+7, y1, sizeof(y1));

                // update party start positions when entering from the west
                const unsigned char x2[8] = { 0x2, 0x2, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0 },
                                    y2[8] = { 0x9, 0x8, 0x9, 0x8, 0x7, 0x9, 0x8, 0x7 };
                memcpy(dungeon->_rooms[i]._partyWestStartX, x2, sizeof(x2));
                memcpy(dungeon->_rooms[i]._partyWestStartY, y2, sizeof(y2));

                // update the map data, moving the chest to the center of the room,
                // and removing the walls at the lower-left corner thereby creating
                // a connection to room 8
                const Coords tile[] = { Coords(5, 5, 0x3C),  // chest
                                        Coords(0, 7, 0x16),  // floor
                                        Coords(1, 7, 0x16),
                                        Coords(0, 8, 0x16),
                                        Coords(1, 8, 0x16),
                                        Coords(0, 9, 0x16) };

                for (j=0; j < int(sizeof(tile)/sizeof(Coords)); j++) {
                    const int index = (tile[j].y * CON_WIDTH) + tile[j].x;
                    dungeon->_rooms[i]._mapData[index] = TileMap::get("base")->translate(tile[j].z);
                }
            }
        }
    }
    u4fclose(dng);

    dungeon->_roomMaps = new CombatMap *[dungeon->_nRooms];
    for (i = 0; i < dungeon->_nRooms; i++)
        initDungeonRoom(dungeon, i);

    return true;
}

/**
 * Loads a dungeon room into map->dungeon->room
 */
void DngMapLoader::initDungeonRoom(Dungeon *dng, int room) {
    dng->_roomMaps[room] = dynamic_cast<CombatMap *>(mapMgr->initMap(Map::COMBAT));

    dng->_roomMaps[room]->_id = 0;
    dng->_roomMaps[room]->_borderBehavior = Map::BORDER_FIXED;
    dng->_roomMaps[room]->_width = dng->_roomMaps[room]->_height = 11;
    dng->_roomMaps[room]->_data = dng->_rooms[room]._mapData; // Load map data
    dng->_roomMaps[room]->_music = Music::COMBAT;
    dng->_roomMaps[room]->_type = Map::COMBAT;
    dng->_roomMaps[room]->_flags |= NO_LINE_OF_SIGHT;
    dng->_roomMaps[room]->_tileset = Tileset::get("base");
}

/**
 * Loads the world map data in from the 'world' file.
 */
bool WorldMapLoader::load(Map *map) {
    U4FILE *world = u4fopen(map->_fname);
    if (!world)
        errorFatal("unable to load map data");

    if (!loadData(map, world))
        return false;

    u4fclose(world);

    return true;
}

} // End of namespace Ultima4
} // End of namespace Ultima
