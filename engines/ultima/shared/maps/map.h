/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you  n redistribute it and/or
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

#ifndef ULTIMA_SHARED_MAPS_MAP_H
#define ULTIMA_SHARED_MAPS_MAP_H

#include "common/array.h"
#include "common/serializer.h"
#include "ultima/shared/core/rect.h"
#include "ultima/shared/maps/map_base.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map_tile.h"


namespace Ultima {
namespace Shared {

class Game;

namespace Maps {

#define REGISTER_WIDGET(NAME) if (name == #NAME) return new Widgets::NAME(_game, this) 
#define DECLARE_WIDGET(NAME) virtual const char *getClassName() const override { return #NAME; }

/**
 * Base class for managing maps within the game
 */
class Map {
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
	MapWidget *createWidget(const Common::String &name) {
		assert(_mapArea);
		return _mapArea->createWidget(name);
	}

	/**
	 * Gets a tile at a given position
	 */
	void getTileAt(const Point &pt, MapTile *tile, bool includePlayer = true) {
		assert(_mapArea);
		return _mapArea->getTileAt(pt, tile, includePlayer);
	}

	/**
	 * Sets a tile at a given position
	 */
	void setTileAt(const Point &pt, uint tileId) {
		assert(_mapArea);
		return _mapArea->setTileAt(pt, tileId);
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

	/**
	 * Cast a specific spell
	 */
	void castSpell(uint spellId) {
		assert(_mapArea);
		_mapArea->castSpell(spellId);
	}
};

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima

#endif
