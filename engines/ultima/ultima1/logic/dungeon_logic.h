
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

#include "ultima/ultima1/data/map_dungeon.h"
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

	/**
	 * Announces any item/monster close enough to the player to be shown in
	 * the dungeon view - a chest or coffin at the player's own feet (only
	 * noticed when standing in a doorway), and any item/monster along the
	 * corridor directly ahead, up to wherever the view is blocked
	 */
	void showNearbyText();

	/**
	 * Get the distance to the closest monster, if any, in front of the player
	 */
	int getMonsterDistance(int maxDistance) const;

	/**
	 * Writes the name of the given monster, special-casing a mimic (which
	 * is otherwise catalogued in UNDERWORLD_MONSTERS under the name of
	 * whatever item it's disguised as)
	 */
	void writeMonsterName(int monsterId);

	/**
	 * Marks the castle quest tied to the given monster (if any) as
	 * completed, provided it's currently outstanding. Only 4 of the 25
	 * monster Ids - one per continent's "kill a monster" quest - have a
	 * quest tied to them
	 */
	void monsterDead(int monsterId);

	/**
	 * Adds coins to the player's purse, capped at 9999
	 */
	void giveCoins(int coins);

	/**
	 * Resolves an attack (physical or magical) landing at the given
	 * dungeon cell - rolls to hit, then applies damage or kills the
	 * monster there, awarding coins/experience on a kill
	 */
	void dungeonAttackAt(int effectNum, int agility, int strike, int x, int y);

	/**
	 * A dungeon monster's melee attack on the player, when it's in an
	 * adjacent cell. deltaX/deltaY point from the monster to the player.
	 * Some monster types have a special effect instead of (or as well as)
	 * plain damage - the gelatinous cube destroys armour, the gremlin eats
	 * food, the mind whipper drains intelligence, the thief pinches a
	 * spare weapon
	 */
	void dungeonMonsterAttack(int deltaX, int deltaY);

	/**
	 * Tries to step the monster at (x, y) by (deltaX, deltaY). Fails on a
	 * wall/secret door/beams tile or an occupied cell. On success the
	 * destination is flagged in `moved` so the grid scan doesn't advance
	 * the same monster again this turn
	 */
	bool dungeonCreatureMove(int x, int y, int deltaX, int deltaY,
		bool moved[][Data::DUNGEON_WIDTH]);

	/**
	 * Returns the magic "strike" power of the currently readied weapon,
	 * used by the Magic Missile spell - a random value up to the player's
	 * intelligence, doubled/tripled/halved again depending on whether a
	 * wand, amulet, staff, or triangle is readied
	 */
	int getMagicWeaponPower();

	/**
	 * Shared finish for the Open/Unlock spells - empties the chest/coffin
	 * at the player's current position and awards coins
	 */
	void castOpen();

	/**
	 * Casts the prayer spell
	 */
	void castPrayer();

	/**
	 * Casts a spell
	 */
	void castSpell(int spell, bool showPhrase);

protected:
	/**
	 * Handle dungeon movement
	 */
	bool move(Data::Direction dir) override;

	/**
	 * Attack command
	 */
	bool attack(Data::Direction dir) override;

	/**
	 * Cast a spell
	 */
	bool cast() override;

	/**
	 * Handles climbing a ladder at the player's current position
	 */
	bool climb() override;

	/**
	 * Unlocks a chest at the player's current cell - an agility check
	 * (auto-passed by wizards); failure springs a trap for dungeonLevel
	 * damage, success awards its coins
	 */
	bool unlock() override;

	/**
	 * Opens a coffin at the player's current cell - a chance something
	 * springs out into the cell ahead, otherwise it yields coins
	 */
	bool open() override;

	/**
	 * Inform/search - reveals a secret door in the direction faced
	 */
	bool inform() override;

	/**
	 * Handles updating creatures/NPCs
	 */
	void updateCreatures() override;

	/**
	 * Called after an action is done
	 */
	void endOfTurn() override;

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
