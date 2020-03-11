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

#ifndef ULTIMA4_DUNGEON_H
#define ULTIMA4_DUNGEON_H

#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

#define DNGROOM_NTRIGGERS 4

enum StatsBonusType {
    STATSBONUS_INT = 0x1,
    STATSBONUS_DEX = 0x2,
    STATSBONUS_STR = 0x4
};

struct Trigger {
    unsigned char tile;
    unsigned char x, y;
    unsigned char change_x1, change_y1, change_x2, change_y2;
};

struct DngRoom {
    Trigger triggers[DNGROOM_NTRIGGERS];
    unsigned char creature_tiles[16];
    unsigned char creature_start_x[16];
    unsigned char creature_start_y[16];
    unsigned char party_north_start_x[8];
    unsigned char party_north_start_y[8];
    unsigned char party_east_start_x[8];
    unsigned char party_east_start_y[8];
    unsigned char party_south_start_x[8];
    unsigned char party_south_start_y[8];
    unsigned char party_west_start_x[8];
    unsigned char party_west_start_y[8];
    MapData  map_data;  // This is OK to change to MapData since sizeof(DngRoom) or 
                        // anything like it is not being used.
    unsigned char buffer[7];
};

/**
 * Dungeon tokens
 */
enum DungeonToken {
    DUNGEON_CORRIDOR            = 0x00,
    DUNGEON_LADDER_UP           = 0x10,
    DUNGEON_LADDER_DOWN         = 0x20,
    DUNGEON_LADDER_UPDOWN       = 0x30,
    DUNGEON_CHEST               = 0x40,
    DUNGEON_CEILING_HOLE        = 0x50,
    DUNGEON_FLOOR_HOLE          = 0x60,    
    DUNGEON_MAGIC_ORB           = 0x70,
    DUNGEON_TRAP                = 0x80,
    DUNGEON_FOUNTAIN            = 0x90,    
    DUNGEON_FIELD               = 0xA0,
    DUNGEON_ALTAR               = 0xB0,
    DUNGEON_DOOR                = 0xC0,
    DUNGEON_ROOM                = 0xD0,
    DUNGEON_SECRET_DOOR         = 0xE0,
    DUNGEON_WALL                = 0xF0
};

class Dungeon : public Map {
public:
    Dungeon() {}

    // Members
    virtual Common::String getName();

    DungeonToken tokenForTile(MapTile tile);
    DungeonToken currentToken();
    unsigned char currentSubToken();
    DungeonToken tokenAt(MapCoords coords);
    unsigned char subTokenAt(MapCoords coords);

    bool ladderUpAt(MapCoords coords);
    bool ladderDownAt(MapCoords coords);

    bool validTeleportLocation(MapCoords coords);

    // Properties
    Common::String name;
    unsigned int n_rooms;
    Std::vector<unsigned char> dataSubTokens;
    DngRoom *rooms;
    CombatMap **roomMaps;
    int currentRoom;
    unsigned char party_startx[8];
    unsigned char party_starty[8];
};

/**
 * Dungeon sub-tokens
 */
enum TrapType {
    TRAP_WINDS                  = 0x0,
    TRAP_FALLING_ROCK           = 0x1,
    TRAP_PIT                    = 0xe
};

enum FountainType {
    FOUNTAIN_NORMAL             = 0x0,
    FOUNTAIN_HEALING            = 0x1,
    FOUNTAIN_ACID               = 0x2,
    FOUNTAIN_CURE               = 0x3,
    FOUNTAIN_POISON             = 0x4
};

enum FieldType {
    FIELD_POISON                = 0x0,
    FIELD_ENERGY                = 0x1,
    FIELD_FIRE                  = 0x2,
    FIELD_SLEEP                 = 0x3
};

void dungeonSearch(void);
void dungeonDrinkFountain();
void dungeonTouchOrb();
bool dungeonHandleTrap(TrapType trap);

bool isDungeon(Map *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
