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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_INTERACTION_H
#define ULTIMA3_VIEWS_INTERACTIONS_INTERACTION_H

#include "graphics/views/view.h"
#include "ultima/ultima3/logic/logic.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

using namespace Graphics::Views;

/**
 * The remainder of a command that needs more input than the key that
 * started it, such as choosing a party member. It reads the keys that follow
 * and writes its prompts and results to the message window
 */
class Interaction {
public:
	virtual ~Interaction() {}

	/**
	 * Returns true if the command was over as soon as it was started
	 */
	virtual bool isFinished() const {
		return false;
	}

	/**
	 * Handles a keypress
	 * @returns		True once the command is finished
	 */
	virtual bool keypress(const KeypressMessage &msg) = 0;
};

/**
 * Asks for a party member by their number
 */
class PlayerChooser {
private:
	int _slot = -1;

public:
	enum Result {
		PENDING,
		CANCELLED,
		CHOSEN
	};

	/**
	 * Handles a keypress. Escape and 0 cancel, and a number with nobody
	 * in that position is turned down
	 */
	Result handleKey(const KeypressMessage &msg);

	/**
	 * Returns the party slot chosen, counting from zero
	 */
	int slot() const {
		return _slot;
	}
};

/**
 * Asks for one of the four directions
 */
class DirectionChooser {
private:
	Direction _dir = DIR_NONE;

public:
	enum Result {
		PENDING,
		CANCELLED,
		CHOSEN
	};

	/**
	 * Handles a keypress, echoing the direction picked
	 */
	Result handleKey(const KeypressMessage &msg);

	Direction direction() const {
		return _dir;
	}
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
