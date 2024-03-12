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

#ifndef MM1_MAPS_MAPS_H
#define MM1_MAPS_MAPS_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

enum TownId {
	NO_TOWN = 0, SORPIGAL = 1, PORTSMITH = 2,
	ALGARY = 3, DUSK = 4, ERLIQUIN = 5
};

enum DirMask {
	DIRMASK_N = 0xC0, DIRMASK_E = 0x30,
	DIRMASK_S = 0xC, DIRMASK_W = 3
};

enum StateFlag {
	SFLAG_SPELLS_DISALLOWED = 2
};

enum Visited {
	VISITED_NONE = 0, VISITED_NORMAL = 1, VISITED_SPECIAL = 2,
	VISITED_EXIT = 3, VISITED_BUSINESS = 4	
};

/**
 * Container for all the game maps
 */
class Maps {
	friend class Map;
private:
	Common::Array<Map *> _maps;
	uint16 _id = 0;
	uint8 _section = 0;
private:
	/**
	 * Gets the index of a map given the id values
	 */
	uint getIndex(uint16 id, byte section);

	/**
	 * Load a map
	 */
	void load(uint mapId);

	/**
	 * Loads tile graphics needed for rendering the 3d view
	 */
	void loadTiles();
	void loadTile();

	/**
	 * Town setup for SORPIGAL & ERLIQUIN
	 */
	void town15setup();

	/**
	 * Town setup for PORTSMITH and ALGARY
	 */
	void town23setup();

	/**
	 * Town setup for DUSK
	 */
	void town4setup();

	/**
	 * Updates masks/offsets based on _forwardMask
	 */
	void updateMasksOffsets();

public:
	Common::Array<Graphics::ManagedSurface> _tiles[3];
	uint _mapId = (uint)-1;
	Common::Point _mapPos;
	uint _mapOffset = 0;
	Map *_currentMap = nullptr;
	byte _currentWalls = 0;
	byte _currentState = 0;
	int _colorOffset = 0;

	DirMask _forwardMask = DIRMASK_N,
		_leftMask = DIRMASK_W,
		_rightMask = DIRMASK_E,
		_backwardsMask = DIRMASK_S;
	int8 _forwardOffset = 0, _leftOffset = 0;
	int8 _rightOffset = 0, _backwardsOffset = 0;
	int _loadId = 0;
	int _loadArea = 0;
	int _loadSection = 0;
	byte _loadFlag = 0;
public:
	Maps();
	~Maps();

	/**
	 * Loads or saves map data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Loads or saves the current map info
	 */
	void synchronizeCurrent(Common::Serializer &s);

	/**
	 * Selects a map
	 */
	void select(uint16 id, byte section);

	/**
	 * Selects a map, and switches to in-game display
	 */
	void display(uint16 id, byte section = 1);

	/**
	 * Loads a town
	 */
	void loadTown(TownId townId);

	/**
	 * Get a given map
	 */
	Map *getMap(uint mapId) const { return _maps[mapId]; }

	/**
	 * Turn left
	 */
	void turnLeft();

	/**
	 * Turn right
	 */
	void turnRight();

	/**
	 * Turn around
	 */
	void turnAround();

	/**
	 * Move a step
	 */
	void step(const Common::Point &delta);

	/**
	 * Called when a map is left to load a new one
	 * and trigger the 3d view to update
	 */
	void changeMap(uint16 id, byte section);

	/**
	 * Mark a tile as visited
	 */
	void visitedTile();

	/**
	 * Disables the special state of the current cell
	 * This is primiarily used for disabling cells
	 * that trigger combat after the first time
	 */
	void clearSpecial();

	/**
	 * Gets the moveement delta given a direction mask
	 */
	static Common::Point getMoveDelta(byte mask);
};

} // namespace Maps

extern Maps::Maps *g_maps;

} // namespace MM1
} // namespace MM

#endif
