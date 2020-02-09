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

#include "ultima/ultima1/widgets/urban_widget.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/maps/creature.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

/**
 * Base class for NPC creatures
 */
class Person : public UrbanWidget, public Shared::Maps::Creature {
	DECLARE_MESSAGE_MAP;
protected:
	Ultima1Game *_game;
	Maps::MapBase *_map;
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
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Person(Ultima1Game *game, Maps::MapBase *map, uint tileNum, int hitPoints) :
		UrbanWidget(game, map, tileNum), Shared::Maps::Creature(game, map, hitPoints), _game(game), _map(map) {}

	/**
	 * Constructor
	 */
	Person(Ultima1Game *game, Maps::MapBase *map, uint tileNum) :
		UrbanWidget(game, map, tileNum), Shared::Maps::Creature(game, map), _game(game), _map(map) {}

	/**
	 * Handles loading and saving data
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Talk to an NPC
	 */
	virtual void talk() {}

	/**
	 * Removes hit points from a creature
	 * @param amount		Amount to remove
	 * @returns				Returns true if kills the creature
	 */
	bool subtractHitPoints(uint amount) override;
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
