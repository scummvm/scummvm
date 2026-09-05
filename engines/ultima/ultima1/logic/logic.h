
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
	void writeString(const char *format, ...);

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
	virtual void endOfTurn();

	/*--- Fallback player action handlers. Each returns true if end of
	 * turn processing should happen immediately afterward, or false if
	 * that's being deferred - typically because a sub-dialog was opened
	 * that will only complete the action once it closes ---*/

	virtual bool board();
	virtual bool cast();
	virtual bool climb();
	virtual bool drop();
	virtual bool enter();
	virtual bool fire();
	virtual bool get();
	virtual bool hyperjump();
	virtual bool inform();
	virtual bool noise();
	virtual bool open();
	virtual bool quit();
	virtual bool ready();
	virtual bool zstats();
	virtual bool move(Data::Direction dir) = 0;
	virtual bool pass();

	/*---------------------------------------*/
public:
	virtual ~Logic() {
	}

	virtual void action(int action);
	virtual void keypress(Common::KeyCode keycode);
	virtual void entering() {
	}
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
