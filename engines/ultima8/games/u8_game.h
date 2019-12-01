/*
Copyright (C) 2004-2005 The Pentagram team

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

#ifndef U8GAME_H
#define U8GAME_H

#include "ultima8/games/game.h"

class IDataSource;

class U8Game: public Game {
public:
	U8Game();
	virtual ~U8Game();

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
	virtual void playQuotes();

	static void ConCmd_cheatItems(const Console::ArgvType &argv);
	static void ConCmd_cheatEquip(const Console::ArgvType &argv);


protected:
	std::string getCreditText(IDataSource *ids);
};


#endif
