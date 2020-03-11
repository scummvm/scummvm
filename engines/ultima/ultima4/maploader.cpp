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

MapLoader *CityMapLoader::instance = MapLoader::registerLoader(new CityMapLoader, Map::CITY);
MapLoader *ConMapLoader::instance = MapLoader::registerLoader(MapLoader::registerLoader(new ConMapLoader, Map::COMBAT), Map::SHRINE);
MapLoader *DngMapLoader::instance = MapLoader::registerLoader(new DngMapLoader, Map::DUNGEON);
MapLoader *WorldMapLoader::instance = MapLoader::registerLoader(new WorldMapLoader, Map::WORLD);

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
	map->data.clear();
	map->data.resize(map->height * map->width);

    if (map->chunk_height == 0)
        map->chunk_height = map->height;
    if (map->chunk_width == 0)
        map->chunk_width = map->width;

    uint32 total = 0;
#ifndef NPERF
    uint32 start = g_system->getMillis();
#endif

    u4fseek(f, map->offset, SEEK_CUR);

    for(ych = 0; ych < (map->height / map->chunk_height); ++ych) {
        for(xch = 0; xch < (map->width / map->chunk_width); ++xch) {
            if (isChunkCompressed(map, ych * map->chunk_width + xch)) {
                MapTile water = map->tileset->getByName("sea")->getId();
                for(y = 0; y < map->chunk_height; ++y) {
                    for(x = 0; x < map->chunk_width; ++x) {
                        map->data[x + (y * map->width) + (xch * map->chunk_width) + (ych * map->chunk_height * map->width)] = water;
                    }
                }
            }
            else {
                for(y = 0; y < map->chunk_height; ++y) {
                    for(x = 0; x < map->chunk_width; ++x) {                    
                        int c = u4fgetc(f);
                        if (c == EOF)
                            return false;
                      
                        uint32 s = g_system->getMillis();
                        MapTile mt = map->translateFromRawTileIndex(c);
                        total += g_system->getMillis() - s;

                        map->data[x + (y * map->width) + (xch * map->chunk_width) + (ych * map->chunk_height * map->width)] = mt;
                    }
                }
            }
        }
    }

    return true;
}

bool MapLoader::isChunkCompressed(Map *map, int chunk) {
    CompressedChunkList::iterator i;    

    for (i = map->compressed_chunks.begin(); i != map->compressed_chunks.end(); i++) {
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

    U4FILE *ult = u4fopen(city->fname);
    U4FILE *tlk = u4fopen(city->tlk_fname);
    if (!ult || !tlk)
        errorFatal("unable to load map data");

    /* the map must be 32x32 to be read from an .ULT file */
    ASSERT(city->width == CITY_WIDTH, "map width is %d, should be %d", city->width, CITY_WIDTH);
    ASSERT(city->height == CITY_HEIGHT, "map height is %d, should be %d", city->height, CITY_HEIGHT);

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
            city->extraDialogues.push_back(dialogues[i]);
        }
    }    

    /*
     * Assign roles to certain people
     */ 
    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        PersonRoleList::iterator current;

        for (current = city->personroles.begin(); current != city->personroles.end(); current++) {
            if ((unsigned)(*current)->id == (i + 1)) {
                if ((*current)->role == NPC_LORD_BRITISH)
                    people[i]->setDialogue(DialogueLoader::getLoader("application/x-u4lbtlk")->load(NULL));
                else if ((*current)->role == NPC_HAWKWIND)
                    people[i]->setDialogue(DialogueLoader::getLoader("application/x-u4hwtlk")->load(NULL));
                people[i]->setNpcType(static_cast<PersonNpcType>((*current)->role));
            }
        }
    }

    /**
     * Add the people to the city structure
     */
    for (i = 0; i < CITY_MAX_PERSONS; i++) {
        if (people[i]->getTile() != 0)            
            city->persons.push_back(people[i]);        
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

    U4FILE *con = u4fopen(map->fname);
    if (!con)
        errorFatal("unable to load map data");

    /* the map must be 11x11 to be read from an .CON file */
    ASSERT(map->width == CON_WIDTH, "map width is %d, should be %d", map->width, CON_WIDTH);
    ASSERT(map->height == CON_HEIGHT, "map height is %d, should be %d", map->height, CON_HEIGHT);

    if (map->type != Map::SHRINE) {
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

    U4FILE *dng = u4fopen(dungeon->fname);
    if (!dng)
        errorFatal("unable to load map data");

    /* the map must be 11x11 to be read from an .CON file */
    ASSERT(dungeon->width == DNG_WIDTH, "map width is %d, should be %d", dungeon->width, DNG_WIDTH);
    ASSERT(dungeon->height == DNG_HEIGHT, "map height is %d, should be %d", dungeon->height, DNG_HEIGHT);

    /* load the dungeon map */
    unsigned int i, j;
    for (i = 0; i < (DNG_HEIGHT * DNG_WIDTH * dungeon->levels); i++) {
        unsigned char mapData = u4fgetc(dng);
        MapTile tile = map->translateFromRawTileIndex(mapData);
        
        /* determine what type of tile it is */
        dungeon->data.push_back(tile);
        dungeon->dataSubTokens.push_back(mapData % 16);
    }

    /* read in the dungeon rooms */
    /* FIXME: needs a cleanup function to free this memory later */
    dungeon->rooms = new DngRoom[dungeon->n_rooms];

    for (i = 0; i < dungeon->n_rooms; i++) {
        unsigned char room_tiles[121];

        for (j = 0; j < DNGROOM_NTRIGGERS; j++) {
            int tmp;

            dungeon->rooms[i].triggers[j].tile = TileMap::get("base")->translate(u4fgetc(dng)).id;

            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->rooms[i].triggers[j].x = (tmp >> 4) & 0x0F;
            dungeon->rooms[i].triggers[j].y = tmp & 0x0F;

            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->rooms[i].triggers[j].change_x1 = (tmp >> 4) & 0x0F;
            dungeon->rooms[i].triggers[j].change_y1 = tmp & 0x0F;
            
            tmp = u4fgetc(dng);
            if (tmp == EOF)
                return false;
            dungeon->rooms[i].triggers[j].change_x2 = (tmp >> 4) & 0x0F;
            dungeon->rooms[i].triggers[j].change_y2 = tmp & 0x0F;
        }

        u4fread(dungeon->rooms[i].creature_tiles, sizeof(dungeon->rooms[i].creature_tiles), 1, dng);
        u4fread(dungeon->rooms[i].creature_start_x, sizeof(dungeon->rooms[i].creature_start_x), 1, dng);
        u4fread(dungeon->rooms[i].creature_start_y, sizeof(dungeon->rooms[i].creature_start_y), 1, dng);
        u4fread(dungeon->rooms[i].party_north_start_x, sizeof(dungeon->rooms[i].party_north_start_x), 1, dng);
        u4fread(dungeon->rooms[i].party_north_start_y, sizeof(dungeon->rooms[i].party_north_start_y), 1, dng);
        u4fread(dungeon->rooms[i].party_east_start_x, sizeof(dungeon->rooms[i].party_east_start_x), 1, dng);
        u4fread(dungeon->rooms[i].party_east_start_y, sizeof(dungeon->rooms[i].party_east_start_y), 1, dng);
        u4fread(dungeon->rooms[i].party_south_start_x, sizeof(dungeon->rooms[i].party_south_start_x), 1, dng);
        u4fread(dungeon->rooms[i].party_south_start_y, sizeof(dungeon->rooms[i].party_south_start_y), 1, dng);
        u4fread(dungeon->rooms[i].party_west_start_x, sizeof(dungeon->rooms[i].party_west_start_x), 1, dng);
        u4fread(dungeon->rooms[i].party_west_start_y, sizeof(dungeon->rooms[i].party_west_start_y), 1, dng);
        u4fread(room_tiles, sizeof(room_tiles), 1, dng);
        u4fread(dungeon->rooms[i].buffer, sizeof(dungeon->rooms[i].buffer), 1, dng);

        /* translate each creature tile to a tile id */
        for (j = 0; j < sizeof(dungeon->rooms[i].creature_tiles); j++)
            dungeon->rooms[i].creature_tiles[j] = TileMap::get("base")->translate(dungeon->rooms[i].creature_tiles[j]).id;

        /* translate each map tile to a tile id */
        for (j = 0; j < sizeof(room_tiles); j++)
            dungeon->rooms[i].map_data.push_back(TileMap::get("base")->translate(room_tiles[j]));

        //
        // dungeon room fixup
        //
        if (map->id == MAP_HYTHLOTH)
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
                memcpy(dungeon->rooms[i].party_east_start_x, x1, sizeof(x1));
                memcpy(dungeon->rooms[i].party_east_start_y, y1, sizeof(y1));

                // update party start positions when entering from the south
                const unsigned char x2[8] = { 0x3, 0x2, 0x3, 0x2, 0x1, 0x3, 0x2, 0x1 },
                                    y2[8] = { 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA };
                memcpy(dungeon->rooms[i].party_south_start_x, x2, sizeof(x2));
                memcpy(dungeon->rooms[i].party_south_start_y, y2, sizeof(y2));
            }
            else if (i == 0x9)
            {
                // update the starting position of monsters 7, 8, and 9
                const unsigned char x1[3] = { 0x4, 0x6, 0x5 },
                                    y1[3] = { 0x5, 0x5, 0x6 };
                memcpy(dungeon->rooms[i].creature_start_x+7, x1, sizeof(x1));
                memcpy(dungeon->rooms[i].creature_start_y+7, y1, sizeof(y1));

                // update party start positions when entering from the west
                const unsigned char x2[8] = { 0x2, 0x2, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0 },
                                    y2[8] = { 0x9, 0x8, 0x9, 0x8, 0x7, 0x9, 0x8, 0x7 };
                memcpy(dungeon->rooms[i].party_west_start_x, x2, sizeof(x2));
                memcpy(dungeon->rooms[i].party_west_start_y, y2, sizeof(y2));

                // update the map data, moving the chest to the center of the room,
                // and removing the walls at the lower-left corner thereby creating
                // a connection to room 8
                const Coords tile[] = { Coords(5, 5, 0x3C),  // chest
                                        Coords(0, 7, 0x16),  // floor
                                        Coords(1, 7, 0x16),
                                        Coords(0, 8, 0x16),
                                        Coords(1, 8, 0x16),
                                        Coords(0, 9, 0x16) };

                for (int j=0; j < int(sizeof(tile)/sizeof(Coords)); j++)
                {
                    const int index = (tile[j].y * CON_WIDTH) + tile[j].x;
                    dungeon->rooms[i].map_data[index] = TileMap::get("base")->translate(tile[j].z);
                }
            }
        }
    }
    u4fclose(dng);

    dungeon->roomMaps = new CombatMap *[dungeon->n_rooms];
    for (i = 0; i < dungeon->n_rooms; i++)
        initDungeonRoom(dungeon, i);

    return true;
}

/**
 * Loads a dungeon room into map->dungeon->room
 */
void DngMapLoader::initDungeonRoom(Dungeon *dng, int room) {
    dng->roomMaps[room] = dynamic_cast<CombatMap *>(mapMgr->initMap(Map::COMBAT));

    dng->roomMaps[room]->id = 0;
    dng->roomMaps[room]->border_behavior = Map::BORDER_FIXED;
    dng->roomMaps[room]->width = dng->roomMaps[room]->height = 11;
    dng->roomMaps[room]->data = dng->rooms[room].map_data; // Load map data
    dng->roomMaps[room]->music = Music::COMBAT;
    dng->roomMaps[room]->type = Map::COMBAT;
    dng->roomMaps[room]->flags |= NO_LINE_OF_SIGHT;
    dng->roomMaps[room]->tileset = Tileset::get("base");
}

/**
 * Loads the world map data in from the 'world' file.
 */
bool WorldMapLoader::load(Map *map) {
    U4FILE *world = u4fopen(map->fname);
    if (!world)
        errorFatal("unable to load map data");

    if (!loadData(map, world))
        return false;

    u4fclose(world);

    return true;
}

} // End of namespace Ultima4
} // End of namespace Ultima
