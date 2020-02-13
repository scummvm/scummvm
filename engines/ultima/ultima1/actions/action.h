/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_ULTIMA1_ACTIONS_ACTION_H
#define ULTIMA_ULTIMA1_ACTIONS_ACTION_H

#include "ultima/shared/actions/action.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;
class GameResources;
namespace Maps {
	class Ultima1Map;
}

namespace Actions {

class Action : public Shared::Actions::Action {
public:
	/**
	 * Constructor
	 */
	Action(TreeItem *parent) : Shared::Actions::Action(parent) {}

	/**
	 * Destructor
	 */
	~Action() override {}

	/**
	 * Jumps up through the parents to find the root game
	 */
	Ultima1Game *getGame();

	/**
	 * Return the game's map
	 */
	Maps::Ultima1Map *getMap();

	/**
	 * Gets the data resources for the game
	 */
	GameResources *getRes();
};

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
