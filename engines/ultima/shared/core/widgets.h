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

#ifndef ULTIMA_SHARED_CORE_WIDGETS_H
#define ULTIMA_SHARED_CORE_WIDGETS_H

#include "ultima/shared/core/map.h"
#include "ultima/shared/gfx/dungeon_surface.h"

namespace Ultima {
namespace Shared {

/**
 * Base class for widgets on maps other than the dungeons
 */
class StandardWidget : public MapWidget {
public:
	/**
	 * Constructor
	 */
	StandardWidget(Game *game, Map::MapBase *map) : MapWidget(game, map) {}
	StandardWidget(Game *game, Map::MapBase *map, const Point &pt) : MapWidget(game, map, pt) {}

	/**
	 * Destructor
	 */
	virtual ~StandardWidget() {}
};

class Monster : public StandardWidget {
protected:
	int _hitPoints;
public:
	/**
	 * Constructor
	 */
	Monster(Game *game, Map::MapBase *map) : StandardWidget(game, map), _hitPoints(0) {}
	Monster(Game *game, Map::MapBase *map, int hitPoints) : StandardWidget(game, map), _hitPoints(hitPoints) {}
	Monster(Game *game, Map::MapBase *map, const Point &pt, int hitPoints) : StandardWidget(game, map, pt), _hitPoints(hitPoints) {}

	/**
	 * Destructor
	 */
	virtual ~Monster() {}
};


/**
 * Base class for things that appear within the dungeons
 */
class DungeonWidget : public MapWidget {
public:
	/**
	* Constructor
	*/
	DungeonWidget(Game *game, Map::MapBase *map) : MapWidget(game, map) {}
	DungeonWidget(Game *game, Map::MapBase *map, const Point &pt) : MapWidget(game, map, pt) {}
	DungeonWidget(Game *game, Map::MapBase *map, const Point &pt, const Common::String &name) :
		MapWidget(game, map, pt, name) {}

	/**
	 * Destructor
	 */
	virtual ~DungeonWidget() {}

	/**
	 * Draws an item
	 */
	virtual void draw(DungeonSurface &s, uint distance) = 0;
	
	/**
	 * Returns true if the given transport type can move to a given position on the map
	 */
	virtual bool canMoveTo(const Point &destPos);
};

/**
 * Base class for dungeon monsters
 */
class DungeonMonster : public DungeonWidget {
protected:
	int _hitPoints;
public:
	/**
	 * Constructor
	 */
	DungeonMonster(Game *game, Map::MapBase *map, int hitPoints) : DungeonWidget(game, map), _hitPoints(hitPoints) {}
	DungeonMonster(Game *game, Map::MapBase *map, const Point &pt, int hitPoints) :
		DungeonWidget(game, map, pt), _hitPoints(hitPoints) {}

	/**
	 * Destructor
	 */
	virtual ~DungeonMonster() {}

	/**
	 * Returns true if a monster blocks the background behind him
	 */
	virtual bool isBlockingView() const = 0;

	/**
	 * Draw a monster
	 */
	virtual void draw(DungeonSurface &s, uint distance) {}
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
