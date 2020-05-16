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

#ifndef ULTIMA4_MAP_DUNGEON_H
#define ULTIMA4_MAP_DUNGEON_H

#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/map.h"

namespace Ultima {
namespace Ultima4 {

#define DNGROOM_NTRIGGERS 4

enum StatsBonusType {
	STATSBONUS_INT = 0x1,
	STATSBONUS_DEX = 0x2,
	STATSBONUS_STR = 0x4
};

struct Trigger {
	byte _tile;
	byte x, y;
	byte _changeX1, _changeY1, changeX2, changeY2;
};

struct PartyEntry {
private:
	Common::Point *_array[5];
public:
	Common::Point _northStart;
	Common::Point _eastStart;
	Common::Point _southStart;
	Common::Point _westStart;

	PartyEntry() {
		_array[DIR_NONE] = nullptr;
		_array[DIR_NORTH] = &_northStart;
		_array[DIR_EAST] = &_eastStart;
		_array[DIR_SOUTH] = &_southStart;
		_array[DIR_WEST] = &_westStart;
	}

	Common::Point &operator[](int dir) {
		return *_array[dir];
	}
};

struct DngRoom {
	Trigger _triggers[DNGROOM_NTRIGGERS];
	byte _creatureTiles[16];
	Common::Point _creatureStart[16];
	PartyEntry _partyStart[8];

	MapData  _mapData;  // This is OK to change to MapData since sizeof(DngRoom) or
	// anything like it is not being used.
	byte _buffer[7];

	/**
	 * Load room data
	 */
	void load(Common::SeekableReadStream &s);

	/**
	 * A couple rooms in hythloth have nullptr player start positions,
	 * which causes the entire party to appear in the upper-left
	 * tile when entering the dungeon room.
	 *
	 * Also, one dungeon room is apparently supposed to be connected
	 * to another, although the the connection does not exist in the
	 * DOS U4 dungeon data file.  This was fixed by removing a few
	 * wall tiles, and relocating a chest and the few monsters around
	 * it to the center of the room.
	 */
	void hythlothFix7();
	void hythlothFix9();
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
	Dungeon();
	~Dungeon() override {}

	// Members
	/**
	 * Returns the name of the dungeon
	 */
	Common::String getName() override;

	/**
	 * Returns the dungeon token associated with the given dungeon tile
	 */
	DungeonToken tokenForTile(MapTile tile);

	/**
	 * Returns the dungeon token for the current location
	 */
	DungeonToken currentToken();

	/**
	 * Returns the dungeon sub-token for the current location
	 */
	byte currentSubToken();

	/**
	 * Returns the dungeon token for the given coordinates
	 */
	DungeonToken tokenAt(MapCoords coords);

	/**
	 * Returns the dungeon sub-token for the given coordinates.  The
	 * subtoken is encoded in the lower bits of the map raw data.  For
	 * instance, for the raw value 0x91, returns FOUNTAIN_HEALING NOTE:
	 * This function will always need type-casting to the token type
	 * necessary
	 */
	byte subTokenAt(MapCoords coords);

	/**
	 * Returns true if a ladder-up is found at the given coordinates
	 */
	bool ladderUpAt(MapCoords coords);

	/**
	 * Returns true if a ladder-down is found at the given coordinates
	 */
	bool ladderDownAt(MapCoords coords);

	bool validTeleportLocation(MapCoords coords);

	// Properties
	Common::String _name;
	uint _nRooms;
	Std::vector<byte> _dataSubTokens;
	DngRoom *_rooms;
	CombatMap **_roomMaps;
	int _currentRoom;
	byte _partyStartX[8];
	byte _partyStartY[8];
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

/**
 * Handles 's'earching while in dungeons
 */
void dungeonSearch();

/**
 * Drink from the fountain at the current location
 */
void dungeonDrinkFountain();

/**
 * Touch the magical ball at the current location
 */
void dungeonTouchOrb();

/**
 * Handles dungeon traps
 */
bool dungeonHandleTrap(TrapType trap);

/**
 * Returns true if 'map' points to a dungeon map
 */
bool isDungeon(Map *punknown);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
