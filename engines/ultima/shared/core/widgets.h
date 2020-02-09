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
	StandardWidget(Game *game, Map::MapBase *map, const Point &pt, Direction dir = DIR_NONE) : MapWidget(game, map, pt, dir) {}

	/**
	 * Destructor
	 */
	~StandardWidget() override {}
};

class Creature : public StandardWidget {
protected:
	int _hitPoints;
	bool _isAttacking;
protected:
	/**
	 * Returns either the maximum attack distance for a monster, or 0 if the monster is beyond
	 * that distance from the player
	 */
	virtual uint attackDistance() const { return 0; }

	/**
	 * Handles moving creatures
	 */
	virtual void movement() {}

	/**
	 * Handles attacks
	 */
	virtual void attack() {}
public:
	/**
	 * Constructor
	 */
	Creature(Game *game, Map::MapBase *map) : StandardWidget(game, map), _hitPoints(0), _isAttacking(false) {}
	Creature(Game *game, Map::MapBase *map, int hitPoints) : StandardWidget(game, map),
		_hitPoints(hitPoints), _isAttacking(false) {}
	Creature(Game *game, Map::MapBase *map, int hitPoints, const Point &pt, Direction dir = DIR_NONE) :
		StandardWidget(game, map, pt, dir), _hitPoints(hitPoints), _isAttacking(false) {}

	/**
	 * Destructor
	 */
	~Creature() override {}

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Called to update the widget at the end of a turn
	 * @param isPreUpdate		Update is called twice in succesion during the end of turn update.
	 *		Once with true for all widgets, then with it false
	 */
	void update(bool isPreUpdate) override;

	/**
	 * True true if the creature is dead
	 */
	bool isDead() const { return _hitPoints <= 0; }
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
	DungeonWidget(Game *game, Map::MapBase *map, const Point &pt, Direction dir = DIR_NONE) : MapWidget(game, map, pt, dir) {}
	DungeonWidget(Game *game, Map::MapBase *map, const Common::String &name, const Point &pt, Direction dir = DIR_NONE) :
		MapWidget(game, map, name, pt, dir) {}

	/**
	 * Destructor
	 */
	~DungeonWidget() override {}

	/**
	 * Draws an item
	 */
	virtual void draw(DungeonSurface &s, uint distance) = 0;
};

/**
 * Stub class for dungeon creatures
 */
class DungeonCreature {
public:
	virtual ~DungeonCreature() {}

	/**
	 * Returns true if a monster blocks the background behind him
	 */
	virtual bool isBlockingView() const = 0;

	/**
	 * Draw a monster
	 */
	virtual void draw(DungeonSurface &s, uint distance) = 0;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
