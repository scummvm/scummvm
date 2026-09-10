
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

class SpaceLogic : public Logic {
private:
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
	 * Sets the cockpit flight speed (1-8), spending 4 fuel per step of
	 * change - the number keys while in the cockpit view. setSpeed
	 */
	void setSpeed(int speed);

protected:
	bool move(Data::Direction dir) override {
		return true;
	}

	/**
	 * Handles updating creatures/NPCs
	 */
	void updateCreatures() override {
	}

	/**
	 * Inform - shows the animated sector scan of the galaxy
	 */
	bool inform() override;

	// Commands that do nothing in outer space - just a beep and a prompt.
	// The rest (board/drop/enter/get/open/steal/transact/unlock/xit/noise)
	// already say the right thing via the base Logic fallbacks
	bool attack(Data::Direction dir) override;
	bool cast() override;
	bool climb() override;
	bool fire() override;
	bool quit() override;
	bool ready() override;
	bool view() override;

public:
	SpaceLogic();
	~SpaceLogic() override {
	}

	void entering() override;

	/**
	 * The number keys set the flight speed while in the cockpit view
	 */
	void keypress(Common::KeyCode keycode) override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
