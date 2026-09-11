
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

#ifndef ULTIMA2_LOGIC_SPACE_LOGIC_H
#define ULTIMA2_LOGIC_SPACE_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * Shared base for the two outer-space modes. Outer space has two quite
 * different play views - the overhead sector map (SpaceMapLogic) and the
 * first-person cockpit (SpaceCockpitLogic) - swapped with the View command.
 * This holds what's common to both: entering(), the fuel accessors, and the
 * many commands that just say "<X>?" in either mode
 */
class SpaceLogic : public Logic {
protected:
	SpaceLogic();

	/**
	 * Fuel remaining in the ship the player is currently flying
	 * (getShipFuel)
	 */
	int shipFuel() const;

	/**
	 * Burns fuel from the current ship, clamped at zero (subtractFuel)
	 */
	void subtractFuel(int amount);

	/**
	 * Shields remaining in the ship the player is currently flying
	 * (getShipShields)
	 */
	int shipShields() const;

	/**
	 * Drains shields from the current ship, clamped at zero
	 * (subtractShields)
	 */
	void subtractShields(int amount);

	/**
	 * Shows the SpaceDead view (death). The caller is expected to have
	 * already written the reason for death to the commands area first
	 */
	void death();

	void updateCreatures() override {
	}

	/**
	 * Space has no land-style turn (no food upkeep, no creature phase) - a
	 * command just refreshes the display
	 */
	void endOfTurn() override;

	/**
	 * Inform - the animated sector scan of the galaxy (works in either view)
	 */
	bool inform() override;

	// Commands that do nothing in outer space - just a beep and a prompt.
	// board/drop/enter/get/open/steal/transact/unlock/xit/noise/pass
	// already say the right thing via the base Logic fallbacks
	bool attack(Data::Direction dir) override;
	bool cast() override;
	bool climb() override;
	bool quit() override;
	bool ready() override;

public:
	~SpaceLogic() override {
	}

	/**
	 * Sets up the whole space encounter: rolls the galaxy, drops the player
	 * at the station sector flying the shuttle. Deliberately on the base
	 * class - it's only ever called on the SpaceMapLogic that take-off
	 * loads, but conceptually it belongs to "entering space", not to the
	 * overhead view specifically
	 */
	void entering() override;

	/**
	 * Any unmapped key is just "Huh?" (overridden by SpaceCockpitLogic for
	 * the speed number keys)
	 */
	void keypress(Common::KeyCode keycode) override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
