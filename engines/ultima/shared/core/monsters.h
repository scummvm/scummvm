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

#ifndef ULTIMA_SHARED_CORE_MONSTERS_H
#define ULTIMA_SHARED_CORE_MONSTERS_H

#include "ultima/shared/core/map.h"
#include "ultima/shared/gfx/dungeon_surface.h"

namespace Ultima {
namespace Shared {

class Monster : public MapWidget {
protected:
	int _hitPoints;
public:
	/**
	 * Constructor
	 */
	Monster(Game *game, Map *map, int hitPoints) : MapWidget(game, map), _hitPoints(hitPoints) {}

	/**
	 * Destructor
	 */
	virtual ~Monster() {}
};

class DungeonMonster : public Monster {
public:
	/**
	 * Constructor
	 */
	DungeonMonster(Game *game, Map *map, int hitPoints) : Monster(game, map, hitPoints) {}

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
	virtual void draw(DungeonSurface &s, uint distance) = 0; 
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
