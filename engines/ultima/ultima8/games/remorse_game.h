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

#include "ultima/ultima8/games/game.h"

namespace Ultima {
namespace Ultima8 {

class RemorseGame : public Game {
public:
	RemorseGame();
	~RemorseGame() override;

	//! load/init game's data files
	bool loadFiles() override;

	//! initialize new game
	bool startGame() override;

	//! start initial usecode
	bool startInitialUsecode(int saveSlot = -1) override;

	//! write game-specific savegame info (avatar stats, equipment, ...)
	void writeSaveInfo(ODataSource *ods) override;

	ProcId playIntroMovie() override;
	ProcId playEndgameMovie() override;
	void playCredits() override;
	void playQuotes() override { };

	static Game *createGame(GameInfo *info);

protected:
	static Game *_game;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
