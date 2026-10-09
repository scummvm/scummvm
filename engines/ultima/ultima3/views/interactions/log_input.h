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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_LOG_INPUT_H
#define ULTIMA3_VIEWS_INTERACTIONS_LOG_INPUT_H

#include "ultima/ultima3/views/interactions/interaction.h"
#include "ultima/ultima3/views/menu_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * A menu answered in the message window. Pressing an option's key writes
 * its name, which then has to be confirmed with Enter or taken back
 */
class LogMenu {
private:
	MenuChoice _menu;
	Common::String _echo;

public:
	/**
	 * Sets up the choices. The word at each index is shown for the key at
	 * the same index, with escape written as 0x1B
	 */
	void setup(const char *keys, const char *const *words, int count);

	/**
	 * Handles a keypress, returning true once a choice has been confirmed
	 */
	bool handleKey(const KeypressMessage &msg);

	/**
	 * Returns the key of the choice made
	 */
	char key() const {
		return _menu.key();
	}
};

/**
 * A yes or no question answered in the message window
 */
class LogYesNo {
private:
	LogMenu _menu;

public:
	LogYesNo();

	/**
	 * Handles a keypress, returning true once an answer has been confirmed
	 */
	bool handleKey(const KeypressMessage &msg) {
		return _menu.handleKey(msg);
	}

	bool yes() const {
		return _menu.key() == 'Y';
	}
};

/**
 * A number typed in the message window, as a couple of digits or as a
 * quantity of up to four. Letters from A to F are taken as digits too
 */
class LogNumber {
private:
	int _maxDigits = 0;
	bool _rejectLetters = false;
	Common::String _text;

	// What was entered, read one digit at a time as BCD
	int _bcd = 0;
	bool _hasLetters = false;

	/**
	 * Works out what has been typed
	 * @returns		False if it isn't a number
	 */
	bool evaluate();

public:
	/**
	 * Starts an entry
	 * @param maxDigits			How many characters can be typed
	 * @param rejectLetters		True if letters are turned down like any other non-number
	 */
	void setup(int maxDigits, bool rejectLetters = false);

	/**
	 * Handles a keypress, returning true once a number has been accepted.
	 * Entries that aren't numbers are turned down
	 */
	bool handleKey(const KeypressMessage &msg);

	/**
	 * Returns what was entered, the digits held as BCD
	 */
	int bcd() const {
		return _bcd;
	}

	/**
	 * Returns what was entered as an ordinary number
	 */
	int value() const {
		return Data::fromBcdWord(_bcd);
	}

	/**
	 * Returns true if letters were used as digits, which gives a value that
	 * can't be relied on
	 */
	bool hasLetters() const {
		return _hasLetters;
	}
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
