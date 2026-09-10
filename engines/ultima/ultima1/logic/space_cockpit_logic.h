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

#ifndef ULTIMA2_LOGIC_SPACE_COCKPIT_LOGIC_H
#define ULTIMA2_LOGIC_SPACE_COCKPIT_LOGIC_H

#include "ultima/ultima1/logic/space_logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * The first-person cockpit view of outer space: a panning starscape where
 * the arrows steer the ship (Left/Right/Climb/Dive), the number keys set
 * the flight speed, and enemy-ship combat happens. Reached from the
 * overhead view with the View command
 */
class SpaceCockpitLogic : public SpaceLogic {
private:
	// tick() fires ~20/sec; only advance the starfield every few calls so
	// it doesn't streak by too fast
	int _tickCounter = 0;

	/**
	 * Sets the flight speed (1-8), spending 4 fuel per step of change
	 * (setSpeed) - the number keys
	 */
	void setSpeed(int speed);

protected:
	bool move(Data::Direction dir) override;
	bool fire() override;
	bool pass() override;
	bool view() override;

public:
	SpaceCockpitLogic() : SpaceLogic() {}
	~SpaceCockpitLogic() override {
	}

	void keypress(Common::KeyCode keycode) override;

	/**
	 * Advances the panning starfield each frame (the starfield half of
	 * cockpitPerFrame)
	 */
	void tick() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
