/*
Copyright (C) 2004-2006 The Pentagram team

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

#include "ultima8/misc/pent_include.h"

#include "ultima8/games/game.h"
#include "U8Game.h"
#include "RemorseGame.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/graphics/palette_manager.h"
#include "ultima8/kernel/kernel.h"
#include "MainMenuProcess.h"

Game *Game::game = 0;

Game::Game() {
	assert(game == 0);
	game = this;
}

Game::~Game() {
	assert(game == this);
	game = 0;
}


// static
Game *Game::createGame(GameInfo *info) {
	switch (info->type) {
	case GameInfo::GAME_U8:
		return new U8Game();
	case GameInfo::GAME_REMORSE:
		return new RemorseGame();
	default:
		CANT_HAPPEN_MSG("createGame: invalid game");
	}

	return 0;
}

uint32 Game::I_playEndgame(const uint8 *args, unsigned int /*argsize*/) {
	SettingManager *settingman = SettingManager::get_instance();
	settingman->set("endgame", true);
	settingman->write();

	PaletteManager *palman = PaletteManager::get_instance();
	palman->untransformPalette(PaletteManager::Pal_Game);

	Process *menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);

	ProcId moviepid = Game::get_instance()->playEndgameMovie();
	Process *movieproc = Kernel::get_instance()->getProcess(moviepid);
	if (movieproc) {
		menuproc->waitFor(movieproc);
	}

	return 0;
}
