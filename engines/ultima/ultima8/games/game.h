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

#ifndef ULTIMA8_GAMES_GAME_H
#define ULTIMA8_GAMES_GAME_H

#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/usecode/intrinsics.h"

namespace Ultima {
namespace Ultima8 {

class Game {
public:
	Game();
	virtual ~Game();

	static Game *get_instance() {
		return _game;
	}

	//! load/init game's data files
	virtual bool loadFiles() = 0;

	//! initialize new game
	virtual bool startGame() = 0;

	//! start initial usecode
	virtual bool startInitialUsecode(int saveSlot = -1) = 0;

	//! write game-specific savegame info (avatar stats, equipment, ...)
	virtual void writeSaveInfo(Common::WriteStream *ws) = 0;

	virtual ProcId playIntroMovie(bool fade) = 0;
	virtual ProcId playEndgameMovie(bool fade) = 0;
	virtual void playCredits() = 0;
	virtual void playQuotes() = 0;

	static Game *createGame(const GameInfo *info);

	INTRINSIC(I_playEndgame);
	INTRINSIC(I_playCredits);

protected:
	static Game *_game;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
