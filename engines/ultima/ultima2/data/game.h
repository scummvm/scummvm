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

#ifndef ULTIMA2_DATA_GAME_H
#define ULTIMA2_DATA_GAME_H

#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

using namespace Shared::Messages;

/*
 * The number of game frames before prompting the user for a command times out,
 * and it's treated as a pass
 */
constexpr int FRAMES_BEFORE_COMMAND_TIMEOUT = 10;

class Game {
public:
	/**
	 * Called to start the game, both for new characters, and when a savegame is loaded
	 */
	void startGame();

	/**
	 * Prompt the player for a command
	 */
	void promptForCommand();

	/**
	 * Implements an in-game command
	 */
	void doCommand(int action);

	/**
	 * Called after actions are done
	 */
	void endOfTurn();

	/**
	 * Subtract food fraction from the player
	 */
	void subtractFoodCtr(int amount);

	/**
	 * Remove HP from the player
	 */
	void subtractHp(int amount);

	/**
	 * Player has died
	 */
	void dead();

	/**
	 * Triggers a message to the game view
	 */
	void message(const Common::String &name, const char *param = nullptr);
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
