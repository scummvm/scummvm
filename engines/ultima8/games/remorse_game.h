/*
Copyright (C) 2006 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_GAMES_REMORSEGAME_H
#define ULTIMA8_GAMES_REMORSEGAME_H

#include "ultima8/games/game.h"

namespace Ultima8 {

class RemorseGame : public Game {
public:
	RemorseGame();
	virtual ~RemorseGame();

	//! load/init game's data files
	virtual bool loadFiles();

	//! initialize new game
	virtual bool startGame();

	//! start initial usecode
	virtual bool startInitialUsecode(const std::string &savegame);

	//! write game-specific savegame info (avatar stats, equipment, ...)
	virtual void writeSaveInfo(ODataSource *ods);

	virtual ProcId playIntroMovie();
	virtual ProcId playEndgameMovie();
	virtual void playCredits();
	virtual void playQuotes() { };

	static Game *createGame(GameInfo *info);

protected:
	static Game *game;
};

} // End of namespace Ultima8

#endif
