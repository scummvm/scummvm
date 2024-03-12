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

#ifndef MM1_MAPS_MAP_H
#define MM1_MAPS_MAP_H

#include "common/array.h"
#include "common/str.h"
#include "mm/mm1/events.h"
#include "mm/mm1/game/game_logic.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MAP_W 16
#define MAP_H 16
#define MAP_SIZE (MAP_W * MAP_H)

class Maps;

enum DataOffset {
	MAP_ID = 0,
	MAP_1 = 1,
	MAP_2 = 2,
	MAP_4 = 4,
	MAP_6 = 6,
	MAP_NORTH_EXIT_ID = 8,
	MAP_NORTH_EXIT_SECTION = 10,
	MAP_EAST_EXIT_ID = 11,
	MAP_EAST_EXIT_SECTION = 13,
	MAP_SOUTH_EXIT_ID = 14,
	MAP_SOUTH_EXIT_SECTION = 16,
	MAP_WEST_EXIT_ID = 17,
	MAP_WEST_EXIT_SECTION = 19,
	MAP_20 = 20,
	MAP_21 = 21,
	MAP_FLEE_THRESHOLD = 22,
	MAP_FLEE_X = 23,
	MAP_FLEE_Y = 24,
	MAP_SURRENDER_THRESHOLD = 25,
	MAP_SURRENDER_X = 26,
	MAP_SURRENDER_Y = 27,
	MAP_BRIBE_THRESHOLD = 28,
	MAP_29 = 29,
	MAP_30 = 30,
	MAP_31 = 31,
	MAP_32 = 32,
	MAP_33 = 33,
	MAP_MAX_MONSTERS = 34,
	MAP_SECTOR1 = 35,
	MAP_SECTOR2 = 36,
	MAP_TYPE = 37,
	MAP_DISPEL_THRESHOLD = 38,
	MAP_SURFACE_ID = 39,
	MAP_SURFACE_SECTION = 41,
	MAP_SURFACE_X = 42,
	MAP_SURFACE_Y = 43,
	MAP_44 = 44,
	MAP_45 = 45,
	MAP_FLAGS = 46,
	MAP_47 = 47,
	MAP_TRAP_THRESHOLD = 48,
	MAP_49 = 49,
	MAP_SPECIAL_COUNT = 50
};

enum WallType {
	WALL_NONE = 0, WALL_NORMAL = 1, WALL_DOOR = 2,
	WALL_TORCH = 3
};

enum CellState {
	CELL_SPECIAL = 0x80, CELL_DARK = 0x20
};

class Maps;

class Map : public Game::GameLogic {
protected:
	Common::String _name;
	Common::String _description;
	uint16 _id;
	uint _mapIndex;
	byte _defaultSection;
	Common::Array<byte> _data;
private:
	/**
	 * Loads the map's maze data
	 */
	void loadMazeData();

	/**
	 * Load the map's overlay file
	 */
	void loadOverlay();
public:
	/**
	 * Divides all the party's Hp in half
	 */
	static void reduceHP();

	/**
	 * Many special methods return a 160 key, but it
	 * doesn't look like the game uses. My current
	 * assumption is that this is just a convenient
	 * 'do nothing' return value
	 */
	static void none160() {}

	/**
	 * Generates an encounter based on the passed id arrays
	 */
	void encounter(const byte *id1, const byte *id2);

public:
	byte _walls[MAP_SIZE];
	byte _states[MAP_SIZE];
	uint8 _visited[MAP_SIZE];
public:
	Map(uint index, const Common::String &name, uint16 id,
		byte defaultSection, const char *desc = nullptr);
	virtual ~Map() {}

	/**
	 * Loads the map
	 */
	virtual void load();

	/**
	 * Handles all special stuff that happens on the map
	 */
	virtual void special() = 0;

	/**
	 * Returns true if mapping is allowed in enhanced mode.
	 * This is to prevent places like the desert where the
	 * players shouldn't be able to see where they are
	 */
	virtual bool mappingAllowed() const {
		return true;
	}

	/**
	 * Gets the map name
	 */
	Common::String getName() const { return _name; }

	/**
	 * Gets the map description for the map display
	 */
	Common::String getDescription() const { return _description; }


	/**
	 * Returns the map Id
	 */
	uint16 getId() const { return _id; }

	/**
	 * Returns the map default section
	 */
	byte getDefaultSection() const {
		return _defaultSection;
	}

	/**
	 * Accesses the map data
	 */
	const byte &operator[](uint ofs) const {
		return _data[ofs];
	}
	byte &operator[](uint ofs) {
		return _data[ofs];
	}
	byte dataByte(uint ofs) const {
		return _data[ofs];
	}
	uint16 dataWord(uint16 ofs) const;
	void dataWord(uint16 ofs, uint16 val);

	/**
	 * Checks whether the party is dead or out of action,
	 * and if so, switches to the death screen
	 */
	bool checkPartyDead();

	/**
	 * Send a message to a UI element
	 */
	template<class T>
	bool send(const T &msg) {
		return g_events->send(msg);
	}
	template<class T>
	bool send(const Common::String &name, const T &msg) {
		return g_events->send(name, msg);
	}

	/**
	 * Returns the map's index
	 */
	uint getMapIndex() const {
		return _mapIndex;
	}

	/**
	 * Updates the game display
	 */
	static void updateGame();

	/**
	 * Redraw the game display
	 */
	static void redrawGame();

	/**
	 * Unlocks door
	 */
	void unlockDoor();

	/**
	 * Visited a special cell
	 */
	void visitedSpecial();

	/**
	 * Visited an exit cell
	 */
	void visitedExit();

	/**
	 * Visited a business cell
	 */
	void visitedBusiness();
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
