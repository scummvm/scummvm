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

#ifndef ULTIMA_ULTIMA1_WIDGETS_PERSON_H
#define ULTIMA_ULTIMA1_WIDGETS_PERSON_H

#include "ultima/ultima1/map/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/widgets.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Base class for NPC creatures
 */
class Person : public Shared::Creature {
private:
	uint _tileNum;
protected:
	Ultima1Game *_game;
	Map::Ultima1Map::MapBase *_map;
protected:
	/**
	 * Returns true if the guards are currently hostile
	 */
	bool areGuardsHostile() const;

	/**
	 * Returns a random movement delta of -1, 0, or 1
	 */
	int getRandomDelta() const;

	/**
	 * Returns a random movement delta
	 */
	Point getRandomMoveDelta() const {
		return Point(getRandomDelta(), getRandomDelta());
	}

	/**
	 * Moves by a given delta if the destination is available
	 * @param delta		Delta to move character by
	 * @returns			True if the move was able to be done
	 */
	bool moveBy(const Point &delta);
public:
	/**
	 * Constructor
	 */
	Person(Ultima1Game *game, Map::Ultima1Map::MapBase *map, uint tileNum, int hitPoints) :
		Shared::Creature(game, map, hitPoints), _game(game), _map(map), _tileNum(tileNum) {}

	/**
	 * Constructor
	 */
	Person(Ultima1Game *game, Map::Ultima1Map::MapBase *map, uint tileNum) :
		Shared::Creature(game, map), _game(game), _map(map), _tileNum(tileNum) {}

	/**
	 * Destructor
	 */
	virtual ~Person() {}

	/**
	 * Get the tile number for the person
	 */
	virtual uint getTileNum() const override { return _tileNum; }

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	virtual CanMove canMoveTo(const Point &destPos);

	/**
	 * Talk to an NPC
	 */
	virtual void talk() {}
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
