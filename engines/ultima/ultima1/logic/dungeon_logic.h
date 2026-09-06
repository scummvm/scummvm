
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

#ifndef ULTIMA2_LOGIC_DUNGEON_LOGIC_H
#define ULTIMA2_LOGIC_DUNGEON_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class DungeonLogic : public Logic {
private:
	/**
	 * Returns the X/Y delta of a single step in the direction the player
	 * is currently facing
	 */
	int getDirDeltaX() const;
	int getDirDeltaY() const;

	/**
	 * Returns true if moving by the given delta from the player's current
	 * position would be blocked - by a wall, secret door, support beam,
	 * a closed door (when already standing on one), or a monster
	 */
	bool isBlocked(int deltaX, int deltaY) const;

	/**
	 * Rotates the player's facing direction 90 degrees left or right,
	 * without moving
	 */
	void turnLeft();
	void turnRight();

	/**
	 * Rotates the player's facing direction 180 degrees, without moving
	 */
	void turnAround();

	/**
	 * Moves the player one step in the direction they're currently
	 * facing, provided the way isn't blocked
	 */
	void moveForward();

	/**
	 * Climbs the ladder at the player's current position up/down a level.
	 * Climbing up from level 1 exits the dungeon back to the overworld
	 */
	void climbLadderUp();
	void climbLadderDown();

protected:
	/**
	 * Handle dungeon movement
	 */
	bool move(Data::Direction dir) override;

	/**
	 * Handles climbing a ladder at the player's current position
	 */
	bool climb() override;

	/**
	 * Handles updating creatures/NPCs
	 */
	void updateCreatures() override;

public:
	DungeonLogic();
	~DungeonLogic() override {
	}

	void entering() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
