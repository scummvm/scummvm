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

#ifndef ULTIMA_SHARED_CORE_MAP_H
#define ULTIMA_SHARED_CORE_MAP_H

#include "common/array.h"
#include "common/ptr.h"
#include "ultima/shared/core/rect.h"

namespace Ultima {
namespace Shared {

enum Direction {
	DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4
};

typedef byte MapCell;

class Game;
class Map;

/**
 * Base class for things that appear within a map, such as monsters, transports, or people
 */
class MapWidget {
protected:
	Game *_game;						// Game reference
	Map *_map;							// Map reference
public:
	Point _position;					// Position within the map
	int _hitPoints;						// Hit pointers
public:
	/**
	 * Constructor
	 */
	MapWidget(Game *game, Map *map) : _game(game), _map(map) {}
	MapWidget() : _game(nullptr), _map(nullptr) {}
	virtual ~MapWidget() {}

	/**
	 * Get the tile for the widget
	 */
	virtual uint getTileNum() const = 0;

	/**
	 * Returns true if the player can move onto a tile the widget occupies
	 */
	virtual bool isBlocking() const { return true; }
};

typedef Common::SharedPtr<MapWidget> MapWidgetPtr;

/**
 * Base class for items that appear within the dungeons
 */
class MapItem {
protected:
	Game *_game;						// Game reference
	Map *_map;							// Map reference
public:
	Point _position;					// Position within the map
public:
	/**
	* Constructor
	*/
	MapItem(Game *game, Map *map) : _game(game), _map(map) {}

	/**
 	 * Destructor
	 */
	virtual ~MapItem() {}

	/**
	 * Draw the item
	 */
	virtual void draw();
};

typedef Common::SharedPtr<MapItem> MapItemPtr;

/**
 * Contains data about a given position within the map
 */
class MapTile {
public:
	int _tileId;							// Tile Id
	int _tileNum;							// Tile number to display. Normally equals Tile Id, but can differ in rare cases
	int _widgetNum;							// Widget number, if any
	MapWidget *_widget;						// Widget pointer
	int _itemNum;							// Item number, if any
	MapItem *_item;							// Item pointer
public:
	/**
	 * Constructor
	 */
	MapTile() : _tileNum(-1), _tileId(-1), _widgetNum(-1), _widget(nullptr), _itemNum(-1), _item(nullptr) {}

	/**
 	 * Destructor
	 */
	virtual ~MapTile() {}

	/**
	 * Clears the map tile information
	 */
	virtual void clear();
};

/**
 * Base class for managing maps within the game
 */
class Map {
	/**
	 * Stores state about the current viewport being displayed. It's kept as part of the Map class
	 * as a convenience to be alongside the current party position
	 */
	struct ViewportPosition {
		Point _topLeft;					// Top, left tile position for viewport
		Point _size;					// Size of the viewport. Just in case we ever allow it to change
		int _mapId;						// Maze the viewport is for. Used to detect when the map changes

		/**
		 * Constructor
		 */
		ViewportPosition() : _topLeft(-1, -1), _mapId(-1) {}

		/**
		 * Returns true if the viewport is in a valid state
		 */
		bool isValid() const { return _mapId != -1; }

		/**
		 * Resets the viewport position, so it'll get recalculated the next call to getViewportPosition
		 */
		void reset() { _mapId = -1; }
	};

	/**
	 * Internal class used for storing the data for a row
	 */
	struct MapCellsRow {
		friend class Map;
	private:
		Common::Array<MapCell> _data;
	public:
		byte &operator[](int idx) { return _data[idx]; }
		byte operator[](int idx) const { return _data[idx]; }
	};
protected:
	byte _mapId;						// The map Id
	Common::Array<MapWidgetPtr> _widgets;	// Party, monsteres, transports, etc.
	Common::Array<MapItemPtr> _items;	// Items like coffins and chests that appear in dungeons
	Common::Array<MapCellsRow> _data;	// Data for the map
	Point _position;					// Current position within the map
	ViewportPosition _viewportPos;		// Viewport position
protected:
	/**
	 * Set the size of the map
	 */
	void setDimensions(const Point &size);
public:
	Point _size;						// X, Y size of the map
	Point _tilesPerOrigTile;			// For enhanced modes, number of tiles per original game tile
	Direction _direction;				// Current direction being faced in the underworld
	bool _fixed;						// Town/city type maps that don't scroll as the player moves
	uint _dungeonLevel;					// Dungeon level number
public:
	/**
	 * Constructor
	 */
	Map();
	virtual ~Map() {}

	/**
	 * Clears all map data
	 */
	virtual void clear();

	/**
	 * Returns the width of the map
	 */
	size_t width() const { return _size.x; }

	/**
	 * Returns the height of the map
	 */
	size_t height() const { return _size.y; }

	/**
	 * Return the current position
	 */
	Point getPosition() const { return _position; }

	/**
	 * Gets a point relative to the current position
	 */
	Point getDeltaPosition(const Point &delta);

	/**
	 * Set the position
	 */
	void setPosition(const Point &pt);

	/**
	 * Get the viewport position
	 */
	Point getViewportPosition(const Point &viewportSize);

	/**
	 * Shifts the viewport by a given delta
	 */
	void shiftViewport(const Point &delta);

	/**
	 * Adds a widget to the map
	 */
	void addWidget(MapWidget *widget);

	/**
	 * Gets a tile at a given position
	 */
	virtual void getTileAt(const Point &pt, MapTile *tile);

	/**
	 * Load a given map
	 */
	virtual void loadMap(int mapId, uint videoMode);

	/**
	 * Returns true if the cell at the given position is a door
	 */
	virtual bool isDoor(const Point &pt) const = 0;

	/**
	 * Returns true if the cell at the given position is a wall or secret door
	 */
	virtual bool isWallOrSecretDoor(const Point &pt) const = 0;

	/**
	 * Returns true if the cell is a type that has walls on it: walls, doors, or secret doors
	 */
	virtual bool isWallOrDoorway(const Point &pt) const = 0;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
