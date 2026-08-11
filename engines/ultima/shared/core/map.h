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

#ifndef ULTIMA_SHARED_CORE_MAP_H
#define ULTIMA_SHARED_CORE_MAP_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "ultima/shared/core/rect.h"
#include "ultima/shared/gfx/dungeon_surface.h"

namespace Ultima {
namespace Shared {

#define REGISTER_WIDGET(NAME) if (name == #NAME) return new Widgets::NAME(_game, (Ultima1Map::MapBase *)map)
#define DECLARE_WIDGET(NAME) const char *getClassName() const override { return #NAME; }

enum Direction {
	DIR_NONE = 0,
	DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4,
	DIR_WEST = 1, DIR_EAST = 2, DIR_NORTH = 3, DIR_SOUTH = 4
};

typedef byte MapCell;
typedef int MapId;

class Game;
class MapWidget;

typedef Common::SharedPtr<MapWidget> MapWidgetPtr;

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
	// Dungeon tile flags
	bool _isDoor, _isSecretDoor;
	bool _isLadderUp, _isLadderDown;
	bool _isWall, _isHallway, _isBeams;
public:
	/**
	 * Constructor
	 */
	MapTile() : _tileNum(-1), _tileId(-1), _widgetNum(-1), _widget(nullptr), _itemNum(-1),
		_isDoor(false), _isSecretDoor(false), _isLadderUp(false), _isLadderDown(false), _isWall(false),
		_isHallway(false), _isBeams(false) {}

	/**
 	 * Destructor
	 */
	virtual ~MapTile() {}

	/**
	 * Clears the map tile information
	 */
	virtual void clear();

	/**
	 * Returns true if the tile is a door in a dungeon
	 */
	bool isDoor() const { return _isDoor; }

	/**
	 * Returns true if the tile is a wall or secret door in a dungeon
	 */
	bool isWallOrSecretDoor() const { return _isWall || _isSecretDoor; }

	/**
	 * Returns true if the tile in a dungeon is a type that has walls on it: walls, doors, or secret doors
	 */
	bool isWallOrDoorway() const { return _isWall || _isDoor || _isSecretDoor; }

	/**
	 * Returns true if a tile is a solid type within a dungeon
	 */
	bool isSolid() const { return !(_isHallway || _isLadderUp || _isLadderDown || _isBeams); }
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
		MapId _mapId;					// Maze the viewport is for. Used to detect when the map changes

		/**
		 * Constructor
		 */
		ViewportPosition() : _topLeft(-1, -1), _mapId(-1) {}

		/**
		 * Returns true if the viewport is in a valid state
		 */
		bool isValid() const { return _mapId != -1; }

		/**
		 * Handles loading and saving viewport
		 */
		void synchronize(Common::Serializer &s) {
			s.syncAsUint16LE(_topLeft.x);
			s.syncAsUint16LE(_topLeft.y);
		}

		/**
		 * Resets the viewport position, so it'll get recalculated the next call to getViewportPosition
		 */
		void reset() { _mapId = -1; }
	};

	/**
	 * Internal class used for storing the data for a row
	 */
	struct MapCellsRow {
	public:
		Common::Array<MapCell> _data;
	public:
		byte &operator[](int idx) { return _data[idx]; }
		byte operator[](int idx) const { return _data[idx]; }
	};
public:
	/**
	 * Base class for specific map types
	 */
	class MapBase {
	private:
		Map *_map;							// Map manager reference
	protected:
		MapId _mapId;						// The map Id
		uint _mapIndex;						// Index of map within the group of same maps
		uint _mapStyle;						// Map style category for towns & castles
		ViewportPosition _viewportPos;		// Viewport position
	protected:
		/**
		 * Set the size of the map
		 */
		void setDimensions(const Point &size);
	public:
		Point _size;						// X, Y size of the map
		Point _tilesPerOrigTile;			// For enhanced modes, number of tiles per original game tile
		Common::String _name;				// Name of map, if applicable
		MapWidget *_playerWidget;		// Current means of transport, even if on foot
		Common::Array<MapWidgetPtr> _widgets;	// Party, monsteres, transports, etc.
		Common::Array<MapCellsRow> _data;	// Data for the map
	public:
		/**
		 * Constructor
		 */
		MapBase(Game *game, Map *map) : _map(map), _playerWidget(nullptr), _mapId(0), _mapIndex(0),
			_mapStyle(0) {}

		/**
		 * Destructor
		 */
		virtual ~MapBase() {}

		/**
		 * Handles loading and saving the map's data
		 */
		virtual void synchronize(Common::Serializer &s);

		/**
		 * Adds a widget to the map
		 */
		void addWidget(MapWidget *widget);

		/**
		 * Removes a widget from the map
		 */
		void removeWidget(MapWidget *widget);

		/**
		 * Clears all map data
		 */
		virtual void clear();

		/**
		 * Gets a tile at a given position
		 */
		virtual void getTileAt(const Point &pt, MapTile *tile);

		/**
		 * Resets the viewport when the viewport changes
		 */
		void resetViewport();

		/**
		 * Get the viewport position
		 */
		virtual Point getViewportPosition(const Point &viewportSize);

		/**
		 * Load the map
		 */
		virtual void load(MapId mapId);

		/**
		 * Changes the level. Only applicable to dungeon maps which have levels
		 * @param delta		Delta to change dungeon level by
		 * @returns			False if dungeon left, true if still within dungeon
		 */
		virtual bool changeLevel(int delta) { return true; }

		/**
		 * Get the current map level
		 */
		virtual uint getLevel() const { return 0; }

		/**
		 * Returns whether the map wraps around to the other side at it's edges (i.e. the overworld)
		 */
		virtual bool isMapWrapped() const { return false; }

		/**
		 * Returns the width of the map
		 */
		size_t width() const { return _size.x; }

		/**
		 * Returns the height of the map
		 */
		size_t height() const { return _size.y; }

		/**
		 * Get the current position
		 */
		Point getPosition() const;

		/**
		 * Set the current position
		 */
		void setPosition(const Point &pt);

		/**
		 * Get the current direction
		 */
		Direction getDirection() const;

		/**
		 * Set the current direction
		 */
		void setDirection(Direction dir);

		/**
		 * Returns a delta for the cell in front of the player based on the direction they're facing
		 */
		Point getDirectionDelta() const;

		/**
		 * Gets a point relative to the current position
		 */
		virtual Point getDeltaPosition(const Point &delta);

		/**
		 * Returns the map Id
		 */
		MapId getMapId() const { return _mapId; }

		/**
		 * Gets the map Index
		 */
		uint getMapIndex() const { return _mapIndex; }

		/**
		 * Shifts the viewport by a given delta
		 */
		virtual void shiftViewport(const Point &delta);

		/**
		 * Updates the map at the end of a turn
		 */
		virtual void update();
	};
protected:
	MapBase *_mapArea;
public:
	/**
	 * Constructor
	 */
	Map() : _mapArea(nullptr) {}

	/**
	 * Destructor
	 */
	virtual ~Map() {}

	/**
	 * Load a given map
	 */
	virtual void load(MapId mapId);

	/**
	 * Clears all map data
	 */
	virtual void clear();

	/**
	 * Handles loading and saving the map's data
	 */
	virtual void synchronize(Common::Serializer &s);

	/**
	 * Instantiates a widget type by name
	 */
	virtual MapWidget *createWidget(Map::MapBase *map, const Common::String &name) = 0;

	/**
	 * Gets a tile at a given position
	 */
	void getTileAt(const Point &pt, MapTile *tile) {
		assert(_mapArea);
		return _mapArea->getTileAt(pt, tile);
	}

	/**
	 * Get the viewport position
	 */
	Point getViewportPosition(const Point &viewportSize) {
		assert(_mapArea);
		return _mapArea->getViewportPosition(viewportSize);
	}

	/**
	 * Return the width of the map
	 */
	size_t width() const {
		assert(_mapArea);
		return _mapArea->width();
	}

	/**
	 * Return the height of the map
	 */
	size_t height() const {
		assert(_mapArea);
		return _mapArea->height();
	}

	/**
	 * Return the current position
	 */
	Point getPosition() const {
		assert(_mapArea);
		return _mapArea->getPosition();
	}

	/**
	 * Set the position
	 */
	void setPosition(const Point &pt) {
		assert(_mapArea);
		_mapArea->setPosition(pt);
	}

	/**
	 * Get the current direction
	 */
	Direction getDirection() const {
		assert(_mapArea);
		return _mapArea->getDirection();
	}

	/**
	 * Set the current direction
	 */
	void setDirection(Direction dir) {
		assert(_mapArea);
		_mapArea->setDirection(dir);
	}

	/**
	 * Returns a delta for the cell in front of the player based on the direction they're facing
	 */
	Point getDirectionDelta() const {
		assert(_mapArea);
		return _mapArea->getDirectionDelta();
	}

	/**
	 * Gets a point relative to the current position
	 */
	Point getDeltaPosition(const Point &delta) {
		assert(_mapArea);
		return _mapArea->getDeltaPosition(delta);
	}

	/**
	 * Shifts the viewport by a given delta
	 */
	void shiftViewport(const Point &delta) {
		assert(_mapArea);
		_mapArea->shiftViewport(delta);
	}

	/**
	 * Returns the number of tiles in the map there are for each tile in the original game.
	 * This allows for more detailed maps in the enhanced game modes
	 */
	Point getTilesPerOrigTile() const {
		assert(_mapArea);
		return _mapArea->_tilesPerOrigTile;
	}

	/**
	 * Return the name of the map
	 */
	Common::String getName() const {
		assert(_mapArea);
		return _mapArea->_name;
	}

	/**
	 * Returns the currently active widget that the player is controlling
	 */
	MapWidget *getPlayerWidget() const {
		assert(_mapArea);
		return _mapArea->_playerWidget;
	}

	/**
	 * @param delta		Delta to change dungeon level by
	 * @returns			False if dungeon left, true if still within dungeon
	 */
	bool changeLevel(int delta) {
		assert(_mapArea);
		return _mapArea->changeLevel(delta);
	}

	/**
	 * Get the current map level
	 */
	uint getLevel() const {
		assert(_mapArea);
		return _mapArea->getLevel();
	}

	/**
	 * Returns whether the map wraps around to the other side at it's edges (i.e. the overworld)
	 */
	bool isMapWrapped() const {
		assert(_mapArea);
		return _mapArea->isMapWrapped();
	}

	/**
	 * Updates the map at the end of a turn
	 */
	void update() {
		assert(_mapArea);
		return _mapArea->update();
	}
};

/**
 * Base class for things that appear within a map, such as monsters, transports, or people
 */
class MapWidget {
protected:
	Game *_game;						// Game reference
	Map::MapBase *_map;					// Map reference
public:
	Point _position;					// Position within the map
	Direction _direction;				// Direction
	Common::String _name;				// Name of widget
public:
	/**
	 * Constructor
	 */
	MapWidget(Game *game, Map::MapBase *map) : _game(game), _map(map) {}
	MapWidget(Game *game, Map::MapBase *map, const Point &pt, Direction dir = DIR_NONE) : _game(game), _map(map), _position(pt), _direction(dir) {}
	MapWidget(Game *game, Map::MapBase *map, const Common::String &name, const Point &pt, Direction dir = DIR_NONE) :
		_game(game), _map(map), _name(name), _position(pt), _direction(dir) {}

	/**
	 * Destructor
	 */
	virtual ~MapWidget() {}

	/**
	 * Return a name for a widget class if it can be synchronized to savegames
	 */
	virtual const char *getClassName() const { return nullptr; }

	/**
	 * Handles loading and saving games
	 */
	virtual void synchronize(Common::Serializer &s);

	/**
	 * Adds a text string to the info area
	 * @param text		Text to add
	 * @param newLine	Whether to apply a newline at the end
	 */
	void addInfoMsg(const Common::String &text, bool newLine = true);

	/**
	 * Get the tile for the widget
	 */
	virtual uint getTileNum() const { return 0; }

	/**
	 * Returns true if the player can move onto a tile the widget occupies
	 */
	virtual bool isBlocking() const { return false; }

	/**
	 * Called to update the widget at the end of a turn
	 * @param isPreUpdate		Update is called twice in succession during the end of turn update.
	 *		Once with true for all widgets, then with it false
	 */
	virtual void update(bool isPreUpdate) {}

	enum CanMove { UNSET = 0, YES = 1, NO = 2 };

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	virtual CanMove canMoveTo(const Point &destPos);

	/**
	 * Moves the widget to a given position
	 * @param destPos		Specified new position
	 * @param dir			Optional explicit direction to set. If not specified,
	 *		the direction will be set relative to the position moved from
	 */
	virtual void moveTo(const Point &destPos, Direction dir = DIR_NONE);
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
