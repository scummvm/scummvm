
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

#ifndef ULTIMA2_LOGIC_CITY_CASTLE_LOGIC_H
#define ULTIMA2_LOGIC_CITY_CASTLE_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class CityCastleLogic : public Logic {
private:
	/**
	 * Checks if the player can move to a given location
	 */
	int checkAt(int x, int y) const;

	/**
	 * Updates Bard/Jesters at end of turn
	 */
	void updateBardJester(int entityIndex);

	/**
	 * Updates wenches at end of turn
	 */
	void updateWench(int entityIndex);

	/**
	 * Updates princesses at end of turn
	 */
	void updatePrincess(int entityIndex);

	/**
	 * Handles hostile guard movement
	 */
	bool guardMove(int entityIndex);

protected:
	/**
	 * Load the NPCs for the given location
	 */
	void loadEntities();

	/**
	 * Handle movement
	 */
	bool move(Data::Direction dir) override;

	/**
	 * Handles updating creatures/NPCs
	 */
	void updateCreatures() override;

	/**
	 * The Drop command is only usable within cities and castles
	 */
	bool drop() override;

	/**
	 * The Get command only does anything within castles - cities just
	 * show a "Get what?" prompt, matching the original
	 */
	bool get() override;

	/**
	 * The Steal command is usable in both cities and castles
	 */
	bool steal() override;

	/**
	 * Checks the castle item allowance, consuming one use if available.
	 * Denies with a message if there isn't one. Used by Get
	 */
	bool checkCastlePermission();

	/**
	 * Rolls whether the player is caught in the act. If so, prints a
	 * message and makes the guards hostile from then on. Wizards get an
	 * extra chance to avoid a middling roll; a hostile guard or a bad
	 * enough roll catches anyone. Used by Steal
	 */
	bool checkCaughtStealing();

	/**
	 * Grants the player a random weapon. If checkCaught is true (Steal),
	 * this can fail by getting the player caught; Get always passes false
	 */
	void findWeapon(bool checkCaught);

	/**
	 * Grants the player a random amount of food. If checkCaught is true
	 * (Steal), this can fail by getting the player caught; Get always
	 * passes false
	 */
	void findFood(bool checkCaught);

	/**
	 * Grants the player a random piece of armor. If checkCaught is true
	 * (Steal), this can fail by getting the player caught; Get always
	 * passes false
	 */
	void findArmor(bool checkCaught);

public:
	~CityCastleLogic() override {
	}

	void entering() override;

	/**
	 * Called to leave the location
	 */
	virtual void leaving();
};

class CityLogic : public CityCastleLogic {
protected:
	/**
	 * The Transact command opens the Merchant view when standing at a shop
	 * counter
	 */
	bool transact() override;

public:
	CityLogic();
	~CityLogic() override {
	}

	void entering() override;
};

class CastleLogic : public CityCastleLogic {
private:
	/**
	 * Returns true if a princess NPC is standing at a given offset from
	 * the player's current position
	 */
	bool isPrincessAtDelta(int deltaX, int deltaY) const;

	/**
	 * Returns true if the player has left the castle with a princess
	 */
	bool isPrincessSaved();

	/**
	 * Called when a princess has been saved, to show the princess dialog before loading the overworld map. 
	 */
	void princessSaved();

protected:
	/**
	 * The Transact command opens the King view when standing at the throne
	 */
	bool transact() override;

public:
	CastleLogic();
	~CastleLogic() override {
	}

	void entering() override;

	/**
	 * Called when leaving the castle
	 */
	void leaving() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
