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

#include "ultima/ultima2/logic/overworld_logic.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

/**
 * Command handling while inside a village, town, or castle. Reuses
 * OverworldLogic's terrain/monster-targeting helpers and combat (attack
 * works identically in a settlement per the original), overriding only
 * what genuinely differs: movement (edge-exit instead of wraparound),
 * monster AI (offerFlag-driven instead of always-hostile, no spawning),
 * and the settlement-only commands.
 */
class CityCastleLogic : public OverworldLogic {
private:
	// Slot targeted by a pending offer(), remembered between the Direction
	// interaction resolving and the OfferGold interaction completing
	int _offerTargetSlot = -1;

	/**
	 * Returns to the overworld position saved when this local map was entered
	 */
	void exitToOverworld();

	/**
	 * Opens the shop/NPC interaction appropriate for the player's current
	 * map quadrant and map type (village/castle vs. town)
	 */
	bool openShopForCurrentQuadrant();

	/**
	 * Handles talking to a Cleric tithe altar
	 */
	bool tithe();

protected:
	void updateCreatures() override;

public:
	~CityCastleLogic() override {}

	bool move(Data::Direction dir) override;
	bool steal(Data::Direction dir) override;
	bool unlock(Data::Direction dir) override;
	bool offer(Data::Direction dir) override;
	bool transact(Data::Direction dir) override;
	void completeOffer(int goldHundreds) override;
};

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima

#endif
