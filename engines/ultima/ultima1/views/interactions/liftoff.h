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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_LIFTOFF_H
#define ULTIMA1_VIEWS_INTERACTIONS_LIFTOFF_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Shown when the player lifts off in a shuttle/time machine to enter the
 * outer space section. Counts down from 10 to 1, one number every second,
 * appending ".." after each but the last - any keypress skips the rest of
 * the countdown. Finishes with "Thou hast lifted off!", then closes and
 * switches to the space map
 */
class LiftOff : public Interaction {
private:
	int _counter = 10;

	/**
	 * Shows the current countdown number, with a trailing ".." if there's
	 * more of the countdown still to come
	 */
	void showCounter();

	/**
	 * Skips the remainder of the countdown
	 */
	void skip();

	/**
	 * Shows the liftoff message, then closes and switches to the space map
	 */
	void finish();

public:
	LiftOff() : Interaction("LiftOff") {
	}
	~LiftOff() override {
	}

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
