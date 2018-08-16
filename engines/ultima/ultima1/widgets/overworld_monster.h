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

#ifndef ULTIMA_ULTIMA1_WIDGETS_OVERWORLD_MONSTER_H
#define ULTIMA_ULTIMA1_WIDGETS_OVERWORLD_MONSTER_H

#include "ultima/shared/core/widgets.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

enum OverworldMonsterId {
	NESS_CREATURE = 0, GIANT_SQUID, DRAGON_TURTLE, PIRATE_SHIP, HOOD, BEAR, HIDDEN_ARCHER, DARK_KNIGHT,
	EVIL_TRENT, THIEF, ORC, KNIGHT, NECROMANCER, EVIL_RANGER, WANDERING_WARLOCK
};

/**
 * Implements monsters on the overworld
 */
class OverworldMonster : public Shared::Creature {
private:
	uint _tileNum;
	OverworldMonsterId _monsterId;
	uint _attackStrength;
protected:
	/**
	 * Returns either the maximum attack distance for a monster, or 0 if the monster is beyond
	 * that distance from the player
	 */
	virtual uint attackDistance() const;


	/**
	 * Handles attacks
	 */
	virtual void attack();

	/**
	 * Handles moving creatures
	 */
	virtual void movement();
public:
	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	static bool canMoveTo(Shared::Map::MapBase *map, MapWidget *widget, const Point &destPos);
public:
	/**
	 * Constructor
	 */
	OverworldMonster(Shared::Game *game, Shared::Map::MapBase *map, uint tileNum, int hitPoints,
		const Point &pt, Shared::Direction dir = Shared::DIR_NONE);

	/**
	 * Destructor
	 */
	virtual ~OverworldMonster() {}

	/**
	 * Returns the monster's type
	 */
	OverworldMonsterId id() const { return _monsterId; }
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
