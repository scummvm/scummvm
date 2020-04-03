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

#ifndef ULTIMA4_CHEAT_H
#define ULTIMA4_CHEAT_H

#include "ultima/ultima4/events/controller.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

class GameController;

class CheatMenuController : public WaitableController<void *> {
public:
	CheatMenuController(GameController *gc);
	bool keyPressed(int key);

private:
	/**
	 * Summons a creature given by 'creatureName'. This can either be given
	 * as the creature's name, or the creature's id.  Once it finds the
	 * creature to be summoned, it calls gameSpawnCreature() to spawn it.
	 */
	void summonCreature(const Common::String &name);

	GameController *_game;
};

/**
 * This class controls the wind option from the cheat menu.  It
 * handles setting the wind direction as well as locking/unlocking.
 * The value field of WaitableController isn't used.
 */
class WindCmdController : public WaitableController<void *> {
public:
	bool keyPressed(int key);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
