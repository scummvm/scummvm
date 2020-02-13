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

#ifndef ULTIMA_SHARED_MAPS_MAP_BASE_H
#define ULTIMA_SHARED_MAPS_MAP_BASE_H

#include "common/array.h"
#include "common/serializer.h"
#include "ultima/shared/core/base_object.h"
#include "ultima/shared/core/rect.h"
#include "ultima/shared/maps/map_widget.h"

namespace Ultima {
namespace Shared {

class Game;

namespace Maps {

typedef int MapId;
typedef byte MapCell;

class Map;
class MapTile;

/**
 * Base class for specific map types
 */
class MapBase {
	/**
	 * Widgets array
	 */
	class WidgetsArray : public Common::Array<MapWidgetPtr> {
	public:
		/**
		 * Finds a widget by class
		 */
		MapWidget *findByClass(const ClassDef &classDef) const;
	};

	/**
	 * Internal class used for storing the data for a row
	 */
	struct MapCellsRow {
	private:
		Common::Array<MapCell> _data;
	public:
		byte &operator[](int idx) { return _data[idx]; }
		byte operator[](int idx) const { return _data[idx]; }

		/**
		 * Resize the row
		 */
		void resize(size_t newSize) { _data.resize(newSize); }
	};

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

private:
//	Map *_map;							// Map manager reference
	Game *_game;						// Game reference
	bool _mapModified;					// Tiles have been dynamically changed
protected:
	MapId _mapId;						// The map Id
	uint _mapIndex;						// Index of map within the group of same maps
	uint _mapStyle;						// Map style category for towns & castles
	ViewportPosition _viewportPos;		// Viewport position
	Common::Array<MapCellsRow> _data;	// Data for the map
protected:
	/**
	 * Set the size of the map
	 */
	void setDimensions(const Point &size);

	/**
	 * Adds a text string to the info area
	 * @param text			Text to add
	 * @param newLine		Whether to apply a newline at the end
	 * @param replaceLine	If true, replaces the current last line
	 */
	void addInfoMsg(const Common::String &text, bool newLine = true, bool replaceLine = false);
public:
	Point _size;						// X, Y size of the map
	Point _tilesPerOrigTile;			// For enhanced modes, number of tiles per original game tile
	Common::String _name;				// Name of map, if applicable
	MapWidget *_playerWidget;			// Current means of transport, even if on foot
	WidgetsArray _widgets;				// Party, monsteres, transports, etc.
public:
	/**
	 * Constructor
	 */
	MapBase(Game *game, Map *) : _game(game),_playerWidget(nullptr),
		_mapModified(false), _mapId(0), _mapIndex(0), _mapStyle(0) {}

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
	virtual void getTileAt(const Point &pt, MapTile *tile, bool includePlayer = true);

	/**
	 * Sets a tile at a given position
	 */
	virtual void setTileAt(const Point &pt, uint tileId);

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
	 * Instantiates a widget type by name
	 */
	virtual MapWidget *createWidget(const Common::String &name) = 0;

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
	 * Gets the map style
	 */
	uint getMapStyle() const { return _mapStyle; }

	/**
	 * Shifts the viewport by a given delta
	 */
	virtual void shiftViewport(const Point &delta);

	/**
	 * Updates the map at the end of a turn
	 */
	virtual void update();

	/**
	 * Cast a specific spell
	 */
	virtual void castSpell(uint spell) = 0;
};

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima

#endif
