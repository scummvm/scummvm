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

#ifndef ULTIMA2_LOGIC_OVERWORLD_LOGIC_H
#define ULTIMA2_LOGIC_OVERWORLD_LOGIC_H

#include "ultima/ultima2/logic/logic.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

/**
 * Command handling while on a planet's overworld.
 */
class OverworldLogic : public Logic {
private:
	/**
	 * Rolls for and, on success, spawns a new monster in a free slot.
	 * Overworld-only - CityCastleLogic doesn't spawn new monsters
	 */
	void trySpawnMonster();

	/**
	 * Stashes the current position as the overworld return point, then
	 * loads the given local map type (1-3 for village/town/castle)
	 */
	void enterLocalMap(int mapType);

	/**
	 * Stashes the current position as the overworld return point, then
	 * enters a tower (4) or dungeon (5) at its first level
	 */
	void enterDungeon(int mapType);

	/**
	 * Counts down the moongate's hop timer, moving it to its next waypoint
	 * when it expires
	 */
	void updatePatrolMarker();

	/**
	 * Steps onto the moongate at (x,y), moving the player to another era
	 */
	void enterMoongate(int x, int y);

	/**
	 * Minax survives a hit - she swaps to the mirror position of her
	 * current spot and becomes permanently passive
	 */
	void minaxFlees(int slot);

	/**
	 * Minax is slain - the endgame victory sequence
	 */
	void minaxDeathSequence();

	/**
	 * A nearby Daemon paralyzes the player's legs, blocking movement until
	 * it wears off, unless the boots resist it
	 */
	void legParalysisTrap();

	/**
	 * A nearby Devil paralyzes the player's arms, blocking attacks until
	 * it wears off, unless the cloak resists it
	 */
	void armParalysisTrap();

	/**
	 * A nearby Mage fires a magic missile
	 */
	void magicMissileTrap();

	/**
	 * A nearby Balron puts the player to sleep, unless the idol resists it
	 */
	void sleepTrap();

protected:
	/**
	 * A nearby Minax curses the player for 1 HP, active or fled, hostile or
	 * not. Returns true if this killed the player
	 */
	bool minaxCurseTrap();

	// STEP_BLOCKED prints no message of its own - the caller shows a
	// generic "--INVALID MOVE!". STEP_PARALYZED already printed its own
	// complete message, so the caller adds nothing further
	enum StepResult { STEP_OK, STEP_BLOCKED, STEP_PARALYZED, STEP_DIED, STEP_MOONGATE };

	/**
	 * Applies the terrain damage, food cost and mount rules for stepping
	 * onto (x,y), printing any resulting messages. Doesn't move the player
	 */
	StepResult stepOnto(int x, int y);

	/**
	 * Returns true if the current mount (or the player on foot) can enter
	 * the given terrain
	 */
	bool mountCanEnter(Data::TileId dest) const;

	/**
	 * Returns -1/0/+1 matching the sign of the low byte of v, replicating
	 * the original's byte-truncating "shortest step on a wrapping map" trick
	 */
	int signByte(int v) const;

	/**
	 * Returns the slot of the active monster at (x,y), or -1 if none
	 */
	int findTargetMonster(int x, int y) const;

	/**
	 * Returns true if any monster, including slot 0, stands at (x,y)
	 */
	bool isOccupied(int x, int y) const;

	/**
	 * Returns true if a monster of the given type can move onto destTile
	 * at (x,y) - not occupied by another monster, and matching its
	 * water/land travel requirement
	 */
	bool monsterCanEnter(Data::TileId monsterTile, Data::TileId destTile, int x, int y) const;

	/**
	 * Attempts to move a monster by the given delta; returns false if blocked
	 */
	bool tryMoveMonster(int slot, int dx, int dy);

	/**
	 * Kills a monster - runs its type's kill-drop table, clears its slot,
	 * and grants the standard gold/experience reward
	 */
	void killMonster(int slot);

	void updateCreatures() override;

public:
	~OverworldLogic() override {}

	/**
	 * Returns true if the given tile can be walked onto on foot
	 */
	bool isWalkable(Data::TileId tile) const;

	bool move(Data::Direction dir) override;
	bool attack(Data::Direction dir) override;
	bool fire(Data::Direction dir) override;
	bool enter() override;
};

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima

#endif
