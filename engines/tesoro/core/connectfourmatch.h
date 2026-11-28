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

#ifndef TESORO_CORE_CONNECTFOURMATCH_H
#define TESORO_CORE_CONNECTFOURMATCH_H

#include "tesoro/core/connectfour.h"
#include "tesoro/core/object.h"

namespace Tesoro {

class World;

/**
 * The game of "Conecta" in the fisherman's hut: the board, whose turn it is, and the story's side of it. The scripts play it
 * through two routines of their own: one starts a game, the other is run over and over while the game is on. The game
 * leaves its result in the script variable 2: 0xff while it goes on, then the value the script had before the game
 * (the player lost or it was a draw) or 2 (the player won).
 */
class ConnectFourMatch {
public:
	enum State { kPlaying = 0, kMachineWon = 1, kPlayerWon = 2 };

	ConnectFour board;
	byte state = kPlaying;
	/** Zero while the winning pieces are hidden. */
	byte blink = 0;
	int32 blinkStamp = 0;
	/** True while a move is playing out: the player's click waits. */
	bool busy = false;
	/** When the player's piece was dropped, or when the game ended; 0 once the machine has been given its turn. */
	int32 moveStamp = 0;

	void start(World &world, byte *variables);
	/** One frame of the game: the machine answers 1.5 seconds after the player, and a finished game ends 3 seconds later. */
	void turn(World &world, byte *variables);

	void showOn(ObjectList &objects, int32 now);

private:
	void finish(World &world, byte result);

	byte _savedResult = 0;
};

} // End of namespace Tesoro

#endif
