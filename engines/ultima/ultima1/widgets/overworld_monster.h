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

#include "ultima/ultima1/widgets/overworld_widget.h"
#include "ultima/shared/maps/creature.h"

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
class OverworldMonster : public OverworldWidget, public Shared::Maps::Creature {
private:
	OverworldMonsterId _monsterId;
	uint _attackStrength;
protected:
	/**
	 * Handles attacking the party
	 */
	void attackParty() override;

	/**
	 * Handles moving creatures
	 */
	void movement() override;
public:
	DECLARE_WIDGET(OverworldMonster)

	/**
	 * Constructor
	 */
	OverworldMonster(Shared::Game *game, Shared::Maps::MapBase *map, uint tileNum, int hitPoints,
		const Point &pt, Shared::Maps::Direction dir = Shared::Maps::DIR_NONE);

	/**
	 * Constructor
	 */
	OverworldMonster(Shared::Game *game, Shared::Maps::MapBase *map) : OverworldWidget(game, map),
		Shared::Maps::Creature(game, map), _monsterId(NESS_CREATURE), _attackStrength(0) {}

	/**
	 * Destructor
	 */
	~OverworldMonster() override {}

	/**
	 * Returns the monster's type
	 */
	OverworldMonsterId id() const { return _monsterId; }

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Returns either the maximum attack distance for a monster, or 0 if the monster is beyond
	 * that distance from the player
	 */
	uint attackDistance() const override;
};

/**
 * Enemy vessels on the overworld
 */
class EnemyVessel : public OverworldMonster {
public:
	/**
	 * Constructor
	 */
	EnemyVessel(Shared::Game *game, Shared::Maps::MapBase *map, uint tileNum, int hitPoints,
		const Point &pt, Shared::Maps::Direction dir = Shared::Maps::DIR_NONE) :
		OverworldMonster(game, map, tileNum, hitPoints, pt, dir) {}

	/**
	 * Constructor
	 */
	EnemyVessel(Shared::Game *game, Shared::Maps::MapBase *map) : OverworldMonster(game, map) {}
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
