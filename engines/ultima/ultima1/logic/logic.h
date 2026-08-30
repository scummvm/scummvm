
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

#ifndef ULTIMA2_LOGIC_H
#define ULTIMA2_LOGIC_H

#include "common/events.h"
#include "ultima/ultima1/data/map.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * Base class for gameplay handling logic used by the Commands class to respond to player actions
 * or other miscellaneous keypresses.
 */
class Logic {
protected:
	/**
	 * Dispatches some text to be shown in the Commands window
	 */
	void writeString(const Common::String &msg);

	/**
	 * Triggers a prompt display in the Commands window
	 */
	void prompt();

	/**
	 * Play a sound effect
	 */
	void playFX(int num);

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber);
	int getRandomNumber(int maxNumber);

	/**
	 * Signal the map to redraw
	 */
	void redrawMap();

	/**
	 * Signal the stats to redraw
	 */
	void redrawStats();

	/**
	 * Handles end of turn logic
	 */
	void endOfTurn();

public:
	virtual ~Logic() {
	}

	virtual void action(int action) {
	}
	virtual void keypress(Common::KeyCode keycode) {
	}
	virtual void enter() {
	}
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
