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

#ifndef ULTIMA_SHARED_WIDGETS_CREATURE_H
#define ULTIMA_SHARED_WIDGETS_CREATURE_H

#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/maps/map.h"
#include "common/serializer.h"

namespace Ultima {
namespace Shared {

class Game;
class Map;

namespace Maps {

/**
 * Base class for creatures that can be killed
 */
class Creature {
private:
	Game *_gameRef;
//	MapBase *_mapRef;
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
	 * Handles attacking the player
	 */
	virtual void attackParty() {}
public:
	/**
	 * Constructor
	 */
	Creature(Game *game, MapBase *) : _gameRef(game), _hitPoints(0), _isAttacking(false) {}
	Creature(Game *game, MapBase *, int hitPoints) : _gameRef(game),
		_hitPoints(hitPoints), _isAttacking(false) {}

	/**
	 * Destructor
	 */
	virtual ~Creature() {}

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Called to update the widget at the end of a turn
	 * @param isPreUpdate		Update is called twice in succession during the end of turn update.
	 *		Once with true for all widgets, then with it false
	 */
	virtual void update(bool isPreUpdate);

	/**
	 * True true if the creature is dead
	 */
	bool isDead() const { return _hitPoints <= 0; }

	/**
	 * Removes hit points from a creature
	 * @param amount		Amount to remove
	 * @returns				Returns true if kills the creature
	 */
	virtual bool subtractHitPoints(uint amount);
};

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima

#endif
