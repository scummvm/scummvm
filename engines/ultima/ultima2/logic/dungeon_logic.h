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

#include "ultima/ultima2/logic/logic.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

/**
 * Command handling inside a tower or dungeon: a first-person maze whose
 * monsters live on individual levels, and whose movement is relative to
 * the direction faced.
 */
class DungeonLogic : public Logic {
private:
	/**
	 * Returns the cell on the current level at a position
	 */
	byte &cellAt(int x, int y) const;

	/**
	 * Returns the slot of the nearest monster on the current level in the
	 * cell straight ahead of the player, or -1
	 */
	int findMonsterAhead() const;

	/**
	 * Steps forward or backward one cell, or turns left/right
	 */
	bool advance();
	bool retreat();
	bool turn(bool left);

	/**
	 * Moves up or down a level, or out to the surface when that goes past
	 * the entrance. Returns true if the level change went ahead
	 */
	void changeLevel(int delta);
	void surface();
	void printLevel();

	/**
	 * Clears a slain monster and hands out its gold and experience
	 */
	void killMonster(int slot);

	/**
	 * Spawns monsters into any empty slots
	 */
	void spawnMonsters();

	/**
	 * Has a monster next to the player attack them. Returns false if that
	 * killed the player
	 */
	bool monsterAttacks(int slot);

	/**
	 * Tries to move a monster to a cell. Returns 0 if the cell is a wall (or
	 * on another level) so another direction is worth trying, 1 if it moved,
	 * and 2 if it's occupied so the monster should just stay put
	 */
	int tryMoveMonster(int slot, int x, int y);

protected:
	void updateCreatures() override;
	bool castSpell(Data::SpellType spell) override;

	bool move(Data::Direction dir) override;
	bool descend() override;
	bool klimb() override;
	bool get() override;

public:
	~DungeonLogic() override {}

	/**
	 * Entering counts as the turn's first, so monsters spawn and the
	 * surroundings are drawn straight away
	 */
	void entering() override {
		endOfTurn();
	}

	bool attack(Data::Direction dir) override;
};

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima

#endif
