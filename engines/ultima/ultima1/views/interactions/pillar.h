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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_PILLAR_H
#define ULTIMA1_VIEWS_INTERACTIONS_PILLAR_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Handles Pillar locations on the overworld map. Each of the 8 pillars
 * shows a one-off inscription and, the first time it's read, either
 * raises one of the player's attributes, grants a random weapon (the
 * "Fortes Fortuna" pillar), or - for the "Go East to go East" pillar -
 * does nothing at all. Visiting an odd-numbered pillar also completes
 * the matching castle's "find a location" quest, if it's currently
 * active
 */
class Pillar : public Interaction {
private:
	int _locationNum = 0;
	byte _borderColor = 0;

	/**
	 * Draws the inscription text for the current pillar
	 */
	void drawInscription();

	/**
	 * Marks the matching castle's quest as completed, if it's currently
	 * outstanding
	 */
	void questCompleted(int locationNum);

	/**
	 * Awards the player a random not-yet-found weapon, if any are left
	 * @return		True if a weapon was found
	 */
	bool findWeapon();

	/**
	 * Raises the given attribute by a tenth or so of the distance to 99
	 * @return		True if the attribute wasn't already maxed out
	 */
	bool gainAttribute(int attrIndex);

	/**
	 * Closes the dialog and returns control to the overworld
	 */
	void finish();

public:
	Pillar() : Interaction("Pillar") {
	}
	~Pillar() override {
	}

	void draw() override;
	bool msgGame(const GameMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void timeout() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
