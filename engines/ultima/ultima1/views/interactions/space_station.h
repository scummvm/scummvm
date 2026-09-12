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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_SPACE_STATION_H
#define ULTIMA1_VIEWS_INTERACTIONS_SPACE_STATION_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Shown when the player's ship comes to rest exactly in one of the space
 * station's docking slots (SpaceMapLogic::tick() - dockAtStation). Charges
 * the 500gp docking fee and refuels the ship that just docked, refusing if
 * the player can't pay; kills the player outright if they aren't wearing a
 * vacuum/reflect suit (space station airlocks don't care); and if more than
 * one real ship is present in the sector, prompts to choose which one to
 * fly out in
 */
class SpaceStation : public Interaction {
private:
	enum State {
		STATE_DONE,
		STATE_CHOOSE_SHIP
	};

	State _state = STATE_DONE;

	/**
	 * Finishes up: refreshes the fuel/shield/coins HUD and hands control
	 * back to the map
	 */
	void finish();

public:
	SpaceStation() : Interaction("SpaceStation") {
	}
	~SpaceStation() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
